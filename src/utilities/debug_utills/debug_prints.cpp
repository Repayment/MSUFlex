#include "utilities/debug_utills/debug_prints.h"

#include <iostream>
#include <cmath>
#include <utilities/reaction.h>

#include "utilities/emu.h"

namespace khnum {
void PrintEmuReaction(const EmuReaction &reaction) {
    // std::cout << "id: " << reaction.id << " ";
    bool is_first = true;
    for (const EmuSubstrate& emu : reaction.left) {
        if (!is_first) {
            std::cout << "+ ";
        }
        PrintEmuSubstrate(emu);
        std::cout << " ";
        is_first = false;
    }
    std::cout << "= ";
    PrintEmuSubstrate(reaction.right);
    //std::cout << "    rate: " << reaction.rate;
    //std::cout << "    id: " << reaction.id;
    std::cout << std::endl;
}

void PrintEmuSubstrate(const EmuSubstrate& emu) {
    //if (fabs(emu.coefficient - 1.0) > 0.0001) {
    //    std::cout << emu.coefficient << " ";
    //}
    PrintEmu(emu.emu);
}


void PrintEmu(const Emu& emu) {
    std::cout << emu.name << ":";
    for (int atom_state : emu.atom_states) {
        std::cout << atom_state;
    }
}


void PrintEmuAndMid(const EmuAndMid& emu_and_mid) {
    PrintEmu(emu_and_mid.emu);
    std::cout << " [";
    for (double fraction : emu_and_mid.mid) {
        std::cout << fraction << ", ";
    }
    std::cout << "]" << std::endl;
}

void PrintVectorOfEmu(const std::vector<Emu>& vec) {
    for (const Emu& emu : vec) {
        PrintEmu(emu);
        std::cout << std::endl;
    }
}

void PrintReactions(const std::vector<Reaction>& reactions) {
    std::cout << "RxnID,rxnEq,rxnCTrans,rates,rxnType,basis,deviation" << std::endl;
    for (const Reaction re : reactions) {
        PrintReaction(re);
    }
}

void PrintReaction(const Reaction& reaction) {
    std::cout << reaction.id << " " << reaction.name << " ";
    size_t left_counter = 0;
    for (Substrate sub : reaction.chemical_equation.left) {
        std::cout << sub.substrate_coefficient_ << " " << sub.name;
        ++left_counter;
        if (left_counter != reaction.chemical_equation.left.size()) {
            std::cout << " + ";
        }
    }
    std::cout << " = ";
    size_t right_counter = 0;
    for (Substrate sub : reaction.chemical_equation.right) {
        std::cout << sub.substrate_coefficient_ << " " << sub.name;
        ++right_counter;
        if (right_counter != reaction.chemical_equation.right.size()) {
            std::cout << " + ";
        }
    }
    std::cout << ", ";
    size_t left_atom_counter = 0;
    for (Substrate sub : reaction.chemical_equation.left) {
        std::cout << sub.atom_coefficient_ << " " << sub.formula;
        ++left_atom_counter;
        if (left_atom_counter != reaction.chemical_equation.left.size()) {
            std::cout << " + ";
        }
    }
    std::cout << " = ";
    size_t right_atom_counter = 0;
    for (Substrate sub : reaction.chemical_equation.right) {
        std::cout << sub.atom_coefficient_ << " " << sub.formula;
        ++right_atom_counter;
        if (right_atom_counter != reaction.chemical_equation.right.size()) {
            std::cout << " + ";
        }
    }
    std::cout << ", ";
    if (reaction.type == ReactionType::Irreversible) {
        std::cout << "F, ";
    }
    if (reaction.type == ReactionType::Forward) {
        std::cout << "FR, ";
    }
    if (reaction.type == ReactionType::Backward) {
        std::cout << "R, ";
    }
    if (reaction.type == ReactionType::IsotopomerBalance) {
        std::cout << "S, ";
    }
    if (reaction.type == ReactionType::MetaboliteBalance) {
        std::cout << "B, ";
    }
    if (!std::isnan(reaction.basis)) {
        std::cout << reaction.basis << ", ";
    } else {
        if (reaction.is_set_free) {
            std::cout << " x, ";
        } else {
            std::cout << "   , ";
        }
    }
    if (!std::isnan(reaction.deviation)) {
        std::cout << reaction.deviation;
    }
    std::cout << std::endl;

}

} // namespace khnum