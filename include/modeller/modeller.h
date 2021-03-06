#pragma once

#include "utilities/problem.h"
#include "utilities/matrix.h"
#include "parser/parser_results.h"


namespace khnum {
class Modeller {
public:
    Modeller(const ParserResults);

    void CalculateInputSubstrateMids();

    void CreateEmuNetworks();

    void CreateNullspaceMatrix();

    void CalculateFluxBounds();

    void CalculateMeasurementsCount();

    void CheckModelForErrors();

    Problem GetProblem();

private:
    std::vector<Reaction> reactions_;
    std::vector<Emu> measured_isotopes_;
    std::vector<Measurement> measurements_;
    std::vector<InputSubstrate> input_substrate_;
    std::vector<std::string> excluded_metabolites_;
    std::vector<int> free_fluxes_id_;

    std::vector<Emu> input_emu_list_;
    std::vector<EmuReaction> all_emu_reactions_;

    Matrix nullspace_;
    Matrix stoichiometry_matrix_;
    std::vector<EmuAndMid> input_substrate_mids_;
    std::vector<EmuNetwork> emu_networks_;
    std::vector<int> id_to_position_in_depended_fluxes_;

    std::vector<double> lower_bounds_;
    std::vector<double> upper_bounds_;

    int measurements_count_ = 0;
};
} // namespace khnum