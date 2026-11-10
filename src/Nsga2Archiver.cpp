#include "Nsga2Archiver.h"
#include <algorithm>
#include <limits>
#include <stdexcept>

namespace UavMoea {

Nsga2Archiver::Nsga2Archiver(uint16_t capacity) : m_capacity(capacity) {
    m_archive.reserve(m_capacity);
}

const std::vector<Individual>& Nsga2Archiver::getArchive() const {
    return m_archive;
}

bool Nsga2Archiver::dominates(const Individual& a, const Individual& b) const {
    bool strictly_better = false;
    for (size_t i = 0; i < a.objectives.size(); ++i) {
        if (a.objectives[i] > b.objectives[i]) {
            return false;
        }
        if (a.objectives[i] < b.objectives[i]) {
            strictly_better = true;
        }
    }
    return strictly_better;
}

std::vector<std::vector<size_t>> Nsga2Archiver::fastNonDominatedSort(const std::vector<Individual>& pop) const {
    std::vector<std::vector<size_t>> fronts;
    std::vector<size_t> domination_counts(pop.size(), 0);
    std::vector<std::vector<size_t>> dominated_lists(pop.size());
    std::vector<size_t> current_front;

    for (size_t p = 0; p < pop.size(); ++p) {
        for (size_t q = 0; q < pop.size(); ++q) {
            if (p == q)
                continue;
            if (dominates(pop[p], pop[q])) {
                dominated_lists[p].push_back(q);
            } else if (dominates(pop[q], pop[p])) {
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

void Nsga2Archiver::calculateCrowdingDistance(std::vector<Individual>& pop, const std::vector<size_t>& front) const {
    size_t size = front.size();
    if (size == 0)
        return;

    for (size_t idx : front) {
        pop[idx].crowding_distance = 0.0;
    }

    size_t num_objectives = pop[front[0]].objectives.size();

    std::vector<size_t> sorted_front = front;

    for (size_t obj = 0; obj < num_objectives; ++obj) {
        std::sort(sorted_front.begin(), sorted_front.end(),
                  [&](size_t a, size_t b) { return pop[a].objectives[obj] < pop[b].objectives[obj]; });

        pop[sorted_front.front()].crowding_distance = std::numeric_limits<double>::infinity();
        pop[sorted_front.back()].crowding_distance = std::numeric_limits<double>::infinity();

        double min_obj = pop[sorted_front.front()].objectives[obj];
        double max_obj = pop[sorted_front.back()].objectives[obj];
        double range = max_obj - min_obj;

        if (range == 0.0)
            continue;

        for (size_t i = 1; i < size - 1; ++i) {
            if (pop[sorted_front[i]].crowding_distance != std::numeric_limits<double>::infinity()) {
                pop[sorted_front[i]].crowding_distance +=
                    (pop[sorted_front[i + 1]].objectives[obj] - pop[sorted_front[i - 1]].objectives[obj]) / range;
            }
        }
    }
}

void Nsga2Archiver::update(const std::vector<Individual>& offspring) {
    if (m_archive.empty()) {
        m_archive = offspring;
        return;
    }

    std::vector<Individual> combined_pop = m_archive;
    combined_pop.insert(combined_pop.end(), offspring.begin(), offspring.end());

    std::vector<std::vector<size_t>> fronts = fastNonDominatedSort(combined_pop);

    std::vector<Individual> next_archive;
    next_archive.reserve(m_capacity);

    size_t current_front_idx = 0;
    while (current_front_idx < fronts.size() && next_archive.size() + fronts[current_front_idx].size() <= m_capacity) {

        calculateCrowdingDistance(combined_pop, fronts[current_front_idx]);

        for (size_t idx : fronts[current_front_idx]) {
            next_archive.push_back(std::move(combined_pop[idx]));
        }
        current_front_idx++;
    }

    if (next_archive.size() < m_capacity && current_front_idx < fronts.size()) {
        std::vector<size_t>& last_front = fronts[current_front_idx];
        calculateCrowdingDistance(combined_pop, last_front);

        std::sort(last_front.begin(), last_front.end(), [&](size_t a, size_t b) {
            return combined_pop[a].crowding_distance > combined_pop[b].crowding_distance;
        });

        size_t needed = m_capacity - next_archive.size();
        for (size_t i = 0; i < needed; ++i) {
            next_archive.push_back(std::move(combined_pop[last_front[i]]));
        }
    }

    m_archive = std::move(next_archive);
}

} // namespace UavMoea