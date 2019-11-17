#include <iostream>
#include "simulator/new_simulator.h"
#include "simulator/simulator_utilities.h"

namespace khnum {
SimulatorResult NewSimulator::CalculateMids(const std::vector<Flux> &fluxes) {
    std::vector<std::vector<Mid>> saved_mids(total_networks_);

    // contains MID's derivative at [free_flux][network][mid]
    std::vector<std::vector<std::vector<Mid>>> saved_diff_mids(total_free_fluxes_);
    for (auto& vec : saved_diff_mids) {
        vec.resize(total_networks_);
    }

    for (int network_num = 0; network_num < total_networks_; ++network_num) {
        SimulatorNetworkData& network = networks_[network_num];
        simulator_utilities::FillFluxMatrix(network.symbolic_A, fluxes, network.A);
        simulator_utilities::FillFluxMatrix(network.symbolic_B, fluxes, network.B);
        simulator_utilities::FillYMatrix(network.Y_data,input_mids_, saved_mids,
                                         network.convolutions, network.Y);
        const Matrix BY = network.B * network.Y;
        const Matrix X = network.A.householderQr().solve(BY);
        simulator_utilities::SaveNewEmus(X, network.usefull_emus, network.final_emus, saved_mids[network_num], simulated_mids_);

        for (int flux = 0; flux < network.derivatives.size(); ++flux) {
            DerivativeData& derivatives = network.derivatives[flux];
            simulator_utilities::FillDiffYMatrix(network.Y_data, saved_diff_mids[flux], network.convolutions,
                                                 input_mids_, saved_mids, derivatives.dY);
            // Right Part of A * dX = (...)
            Matrix RightPart = derivatives.dB * network.Y + network.B * derivatives.dY - derivatives.dA * X;
            Matrix dX = network.A.householderQr().solve(RightPart);
            simulator_utilities::SaveNewEmus(dX, network.usefull_emus, network.final_emus,
                                             saved_diff_mids[flux][network_num], diff_results_[flux]);
        }
    }

    SimulatorResult result;
    result.simulated_mids = simulated_mids_;
    result.diff_results = diff_results_;
    return result;
}

NewSimulator::NewSimulator(const std::vector<SimulatorNetworkData>& networks,
                           const std::vector<EmuAndMid>& input_mids,
                           const size_t total_free_fluxes) : total_networks_{networks.size()},
                                               total_free_fluxes_{total_free_fluxes},
                                               input_mids_{input_mids},
                                               networks_{networks} {
    simulated_mids_.resize(input_mids.size());
    diff_results_.resize(total_free_fluxes);
    for (std::vector<EmuAndMid>& vec : diff_results_) {
        vec.resize(input_mids.size());
    }
}
} // namespace khnum