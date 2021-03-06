#include "modeller/create_network_components.h"

#include <algorithm>
#include <vector>
#include <iostream>

#include "utilities/emu.h"
#include "utilities/debug_utills/debug_prints.h"

namespace khnum {
    namespace modelling_utills {
        std::vector<EmuNetwork> CreateNetworkComponents(const EmuNetwork &network) {
            std::vector<EmuNetwork> components;

            std::vector<Emu> all_emus;

            for (const EmuReaction &reaction : network) {
                if (reaction.left.size() == 1) {
                    if (std::find(all_emus.begin(), all_emus.end(), reaction.left[0].emu) == all_emus.end()) {
                        all_emus.push_back(reaction.left[0].emu);
                    }
                }

                if (std::find(all_emus.begin(), all_emus.end(), reaction.right.emu) == all_emus.end()) {
                    all_emus.push_back(reaction.right.emu);
                }
            }

            std::vector<size_t> ordered_substrates;
            std::vector<char> visited(all_emus.size(), false);
            for (size_t v = 0; v < all_emus.size(); ++v) {
                if (!visited[v]) {
                    dfs1(v, network, visited, ordered_substrates, all_emus);
                }
            }

            visited.assign(all_emus.size(), false);
            for (int i = 0; i < all_emus.size(); ++i) {
                int v = ordered_substrates[all_emus.size() - 1 - i];
                if (!visited[v]) {
                    std::vector<size_t> vertices_of_component;
                    dfs2(v, network, visited, vertices_of_component, all_emus);
                    EmuNetwork component;
                    for (const EmuReaction &reaction : network) {
                        for (size_t v : vertices_of_component) {
                            if (reaction.right.emu == all_emus[v]) {
                                component.push_back(reaction);
                                break;
                            }
                        }
                    }
                    if (!component.empty()) {
                        components.push_back(component);
                    }
                }
            }
            return components;
        }

        void dfs1(int v, const EmuNetwork &network, std::vector<char> &visited, std::vector<size_t> &ordered_reactions, const std::vector<Emu> &all_emus) {
            visited[v] = true;
            for (int i = 0; i < network.size(); ++i) {
                if (network.at(i).left.size() == 1 && network.at(i).left[0].emu == all_emus.at(v)) {
                    auto right = std::find(all_emus.begin(), all_emus.end(), network.at(i).right.emu);
                    if (!visited[right - all_emus.begin()]) {
                        dfs1(right - all_emus.begin(), network, visited, ordered_reactions, all_emus);
                    }
                }
            }
            ordered_reactions.push_back(v);
        }

        void dfs2(int v, const EmuNetwork &emu_network, std::vector<char> &visited, std::vector<size_t> &component, const std::vector<Emu> &all_emus) {
            visited[v] = true;
            component.push_back(v);
            for (size_t i = 0; i < emu_network.size(); ++i) {
                if (emu_network[i].right.emu == all_emus[v] && emu_network[i].left.size() == 1) {
                    auto left = std::find(all_emus.begin(), all_emus.end(), emu_network[i].left[0].emu);
                    if (!visited[left - all_emus.begin()]) {
                        dfs2(left - all_emus.begin(), emu_network, visited, component, all_emus);
                    }
                }
            }
        }
    }
}