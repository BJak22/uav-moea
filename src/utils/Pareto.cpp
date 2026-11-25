#include "utils/Pareto.h"

namespace UavMoea::utils {

std::vector<std::vector<size_t>> fastNonDominatedSort(const std::vector<Individual>& population) {
    const size_t n = population.size();
    if (n == 0) {
        return {};
    }

    std::vector<std::vector<size_t>> fronts;
    std::vector<size_t> domination_counts(n, 0);
    std::vector<size_t> dominated_flat(n * n);
    std::vector<size_t> dominated_sizes(n, 0);
    std::vector<size_t> current_front;
    current_front.reserve(n);

    for (size_t p = 0; p < n; ++p) {
        size_t* p_list = &dominated_flat[p * n];
        size_t p_count = 0;
        for (size_t q = 0; q < n; ++q) {
            if (p == q) {
                continue;
            }
            if (population[p].dominates(population[q])) {
                p_list[p_count++] = q;
            } else if (population[q].dominates(population[p])) {
                domination_counts[p]++;
            }
        }
        dominated_sizes[p] = p_count;
        if (domination_counts[p] == 0) {
            current_front.push_back(p);
        }
    }

    fronts.push_back(std::move(current_front));

    size_t front_idx = 0;
    while (!fronts[front_idx].empty()) {
        std::vector<size_t> next_front;
        for (size_t p : fronts[front_idx]) {
            const size_t* p_list = &dominated_flat[p * n];
            const size_t p_count = dominated_sizes[p];
            for (size_t i = 0; i < p_count; ++i) {
                const size_t q = p_list[i];
                if (--domination_counts[q] == 0) {
                    next_front.push_back(q);
                }
            }
        }
        if (next_front.empty()) {
            break;
        }
        fronts.push_back(std::move(next_front));
        front_idx++;
    }

    return fronts;
}

} // namespace UavMoea::utils