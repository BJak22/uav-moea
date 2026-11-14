#include "utils/Pareto.h"

namespace UavMoea::utils {

std::vector<std::vector<size_t>> fastNonDominatedSort(const std::vector<Individual>& population) {
    std::vector<std::vector<size_t>> fronts;
    std::vector<size_t> domination_counts(population.size(), 0);
    std::vector<std::vector<size_t>> dominated_lists(population.size());
    std::vector<size_t> current_front;

    for (size_t p = 0; p < population.size(); ++p) {
        for (size_t q = 0; q < population.size(); ++q) {
            if (p == q)
                continue;
            if (population[p].dominates(population[q])) {
                dominated_lists[p].push_back(q);
            } else if (population[q].dominates(population[p])) {
                domination_counts[p]++;
            }
        }
        if (domination_counts[p] == 0) {
            current_front.push_back(p);
        }
    }

    fronts.push_back(current_front);

    size_t front_idx = 0;
    while (!fronts[front_idx].empty()) {
        std::vector<size_t> next_front;
        for (size_t p : fronts[front_idx]) {
            for (size_t q : dominated_lists[p]) {
                domination_counts[q]--;
                if (domination_counts[q] == 0) {
                    next_front.push_back(q);
                }
            }
        }
        if (next_front.empty())
            break;
        fronts.push_back(next_front);
        front_idx++;
    }

    return fronts;
}

} // namespace UavMoea::utils