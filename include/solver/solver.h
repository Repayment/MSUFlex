#pragma once

#include "alglib/ap.h"
#include <random>

#include "alglib/optimization.h"
#include "utilities/problem.h"
#include "simulator/simulator.h"
#include "simulator/generator.h"


namespace khnum {
// See http://www.alglib.net/optimization/levenbergmarquardt.php
// and http://www.alglib.net/translator/man/manual.cpp.html#example_minlm_d_v before reading this code

class Solver {
public:
    Solver(const Problem &problem, const SimulatorGenerator &generator);

    void Solve();

    std::vector<alglib::real_1d_array> GetResult();

private:
    void SetOptimizationParameters();

    // Set constraints so the fluxes always > 0
    void SetConstraints();

    void GenerateInitialPoints(std::mt19937 &random_source);

    void PrintStartMessage();

    alglib::real_1d_array RunOptimization();

    void CalculateResidual(const alglib::real_1d_array &free_fluxes,
                           alglib::real_1d_array &residuals);

    std::vector<Flux> CalculateAllFluxesFromFree(const alglib::real_1d_array &free_fluxes_alglib);

    void Fillf0Array(alglib::real_1d_array &residuals, const std::vector<EmuAndMid> &simulated_mids);

    double GetSSR(const alglib::real_1d_array &residuals);

    void PrintFinalMessage(const alglib::real_1d_array &free_fluxes);

    friend void AlglibCallback(const alglib::real_1d_array &free_fluxes,
                               alglib::real_1d_array &residuals, void *ptr);

    friend void JacobianCallback(const alglib::real_1d_array &free_fluxes,
                                      alglib::real_1d_array &fi,
                                      alglib::real_2d_array &jac, void *ptr);

    void FillJacobian(alglib::real_2d_array &jac);

public:
    bool in_jacobian = false;
    int iteration_;
    alglib::real_1d_array free_fluxes_;
    alglib::minlmstate state_;
    alglib::minlmreport report_;
    std::vector<alglib::real_1d_array> all_solutions_;
    std::optional<Simulator> new_simulator_;
    std::vector<std::vector<EmuAndMid>> diff_results_;

    int nullity_;
    int reactions_num_;
    int measurements_count_;
    int iteration_total_;
    bool use_analytic_gradient_ = false;
    std::vector<ReactionsName> reactions_;

    Matrix nullspace_;
    std::vector<Measurement> measured_mids_;

    alglib::real_1d_array lower_bounds_;
    alglib::real_1d_array upper_bounds_;
};

void AlglibCallback(const alglib::real_1d_array &free_fluxes,
                    alglib::real_1d_array &residuals, void *ptr);

void JacobianCallback(const alglib::real_1d_array &free_fluxes,
                      alglib::real_1d_array &fi,
                      alglib::real_2d_array &jac, void *ptr);
} // namespace khnum