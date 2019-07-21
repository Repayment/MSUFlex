#include "solver.h"

#include <random>
#include "alglib/optimization.h"

#include "utilities/get_eigen_vec_from_alglib_vec.h"


namespace khnum {


Solver::Solver(const Problem &problem) :
            simulator_(problem.networks, problem.input_mids, problem.measured_isotopes) {
    reactions_ = problem.reactions;
    nullspace_ = problem.nullspace;
    measurements_ = problem.measurements;
    measurements_count_ = problem.measurements_count;

    nullity_ = nullspace_.cols();
    free_fluxes_.setlength(nullity_);
    lower_bounds_.setlength(nullity_);
    upper_bounds_.setlength(nullity_);

    iteration_ = 0;
    iteration_total_ = 10;
    reactions_num_ = reactions_.size();
}


std::vector<alglib::real_1d_array> Solver::getResult() {
    return all_solutions_;
}


void Solver::Solve() {
    FillBoundVectors();
    SetOptimizationParameters();

    std::random_device randomizer;
    std::mt19937 random_source(randomizer());

    for (iteration_ = 0; iteration_ < iteration_total_; ++iteration_) {
        GenerateInitialPoints(random_source);
        alglib::minlmrestartfrom(state_, free_fluxes_);
        alglib::real_1d_array new_solution = RunOptimization();

        all_solutions_.emplace_back(new_solution);
    }
}


void Solver::FillBoundVectors() {
    for (int i = 0; i < nullity_; ++i) {
        lower_bounds_[i] = reactions_[reactions_num_ - nullity_ + i].computed_lower_bound;
        upper_bounds_[i] = reactions_[reactions_num_ - nullity_ + i].computed_upper_bound;
    }
}


void Solver::SetOptimizationParameters() {
    alglib::ae_int_t maxits = 0;
    const double epsx = 0.00000000001;

    alglib::minlmcreatev(nullity_, measurements_count_, free_fluxes_, 0.0001, state_);
    alglib::minlmsetcond(state_, epsx, maxits);
    alglib::minlmsetbc(state_, lower_bounds_, upper_bounds_);
}


void Solver::GenerateInitialPoints(std::mt19937 &random_source) {
    std::uniform_real_distribution<> get_random_point(0.0, 1.0);

    for (int i = 0; i < nullity_; ++i) {
        free_fluxes_[i] = lower_bounds_[i] + get_random_point(random_source) * (upper_bounds_[i] - lower_bounds_[i]);
    }

    PrintStartMessage();
}


void Solver::PrintStartMessage() {
    std::cout << "Start " << iteration_ << " iteration from: " << std::endl;
    for (int i = 0; i < nullity_; ++i) {
        std::cout << reactions_[reactions_num_ - nullity_ + i].name << " = " << free_fluxes_[i] << std::endl;
    }
    std::cout << std::endl;
}


alglib::real_1d_array Solver::RunOptimization() {
    alglib::minlmoptimize(state_, AlglibCallback, nullptr, this);

    alglib::real_1d_array final_free_fluxes;
    alglib::minlmresults(state_, final_free_fluxes, report_);

    PrintFinalMessage(final_free_fluxes);

    return final_free_fluxes;
}


void AlglibCallback(const alglib::real_1d_array &free_fluxes,
                    alglib::real_1d_array &residuals, void *ptr) {
    Solver* solver = static_cast<Solver*>(ptr);
    solver->CalculateResidual(free_fluxes, residuals);
}

void Solver::CalculateResidual(const alglib::real_1d_array &free_fluxes,
                               alglib::real_1d_array &residuals) {
    std::vector<Flux> calculated_fluxes = CalculateAllFluxesFromFree(free_fluxes);
    std::vector<EmuAndMid> simulated_mids = simulator_.CalculateMids(calculated_fluxes);
    Fillf0Array(residuals, simulated_mids);
}


std::vector<Flux> Solver::CalculateAllFluxesFromFree(const alglib::real_1d_array &free_fluxes_alglib) {
    Eigen::VectorXd free_fluxes = GetEigenVectorFromAlgLibVector(free_fluxes_alglib);
    Matrix depended_fluxes_matrix = -nullspace_ * free_fluxes;
    std::vector<Flux> all_fluxes(reactions_num_);
    // non metabolite balance reactions
    const int depended_reactions_total = depended_fluxes_matrix.rows();
    const int
        metabolite_balance_reactions_total = reactions_num_ - depended_reactions_total - free_fluxes_alglib.length();

    for (int i = 0; i < metabolite_balance_reactions_total; ++i) {
        all_fluxes[reactions_.at(i).id] = 1;
    }

    for (int i = 0; i < depended_reactions_total; ++i) {
        all_fluxes[reactions_.at(i + metabolite_balance_reactions_total).id] = depended_fluxes_matrix(i, 0);
    }

    for (int i = 0; i < free_fluxes_alglib.length(); ++i) {
        all_fluxes[reactions_.at(reactions_num_ - free_fluxes_alglib.length() + i).id] = free_fluxes[i];
    }

    return all_fluxes;
}


void Solver::Fillf0Array(alglib::real_1d_array &residuals, const std::vector<EmuAndMid> &simulated_mids) {
    int total_residuals = 0;
    for (int isotope = 0; isotope < simulated_mids.size(); ++isotope) {
        for (int mass_shift = 0; mass_shift < simulated_mids[isotope].mid.size(); ++mass_shift) {
            residuals[total_residuals] = simulated_mids[isotope].mid[mass_shift];
            residuals[total_residuals] -= (measurements_[isotope].mid[mass_shift]);
            residuals[total_residuals] /= 1 + measurements_[isotope].errors[mass_shift];
            ++total_residuals;
        }
    }
}


double Solver::GetSSR(const alglib::real_1d_array &residuals) {
    double answer = 0.0;
    for (int measurement = 0; measurement < measurements_count_; ++measurement) {
        answer += residuals(measurement) * residuals(measurement);
    }
    return answer;
}


void Solver::PrintFinalMessage(const alglib::real_1d_array &free_fluxes) {
    std::vector<Flux> final_all_fluxes = CalculateAllFluxesFromFree(free_fluxes);
    std::vector<EmuAndMid> simulated_mids = simulator_.CalculateMids(final_all_fluxes);
    alglib::real_1d_array residuals;
    residuals.setlength(measurements_count_);
    Fillf0Array(residuals, simulated_mids);
    double ssr = GetSSR(residuals);

    std::cout << "Finish at: " << std::endl;

    for (int i = 0; i < free_fluxes.length(); ++i) {
        std::cout << reactions_[reactions_num_ - free_fluxes.length() + i].name <<
                  " = " << free_fluxes[i] << std::endl;
    }
    std::cout << "with SSR: " << ssr << " in " << report_.iterationscount << " steps." << std::endl << std::endl;
}
} // namespace khnum