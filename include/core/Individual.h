#pragma once

#include <any>
#include <vector>

namespace UavMoea {

struct Individual {
    std::vector<double> objectives;
    std::any genotype;

    bool dominates(const Individual& other) const {
        bool strictly_better = false;
        for (size_t i = 0; i < objectives.size(); ++i) {
            if (objectives[i] > other.objectives[i]) {
                return false;
            }
            if (objectives[i] < other.objectives[i]) {
                strictly_better = true;
            }
        }
        return strictly_better;
    }
};

} // namespace UavMoea