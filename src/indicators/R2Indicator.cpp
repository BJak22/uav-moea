#include "indicators/R2Indicator.h"

#include <algorithm>
#include <cmath>
#include <limits>

namespace UavMoea::indicators {

R2Indicator::R2Indicator(std::vector<std::vector<double>> weights, std::vector<double> ideal_point)
    : m_weights(std::move(weights)), m_ideal_point(std::move(ideal_point)) {}

void R2Indicator::updateState(const std::vector<Individual>& population) {
    if (population.empty())
        return;

    if (m_ideal_point.empty()) {
        m_ideal_point.resize(population[0].objectives.size(), std::numeric_limits<double>::infinity());
    }

    for (const auto& ind : population) {
        for (size_t i = 0; i < ind.objectives.size(); ++i) {
            if (ind.objectives[i] < m_ideal_point[i]) {
                m_ideal_point[i] = ind.objectives[i];
            }
        }
    }
}

double R2Indicator::evaluate(const std::vector<Individual>& population) const {
    if (population.empty())
        return std::numeric_limits<double>::infinity();

    double total_r2 = 0.0;

    for (const auto& weight : m_weights) {
        double min_tchebycheff = std::numeric_limits<double>::infinity();

        for (const auto& ind : population) {
            double max_diff = -std::numeric_limits<double>::infinity();

            for (size_t i = 0; i < ind.objectives.size(); ++i) {
                double diff = weight[i] * std::abs(ind.objectives[i] - m_ideal_point[i]);
                if (diff > max_diff) {
                    max_diff = diff;
                }
            }
            if (max_diff < min_tchebycheff) {
                min_tchebycheff = max_diff;
            }
        }
        total_r2 += min_tchebycheff;
    }

    return total_r2 / m_weights.size();
}

std::vector<double> R2Indicator::calculateContributions(const std::vector<Individual>& population) const {
    std::vector<double> contributions(population.size(), 0.0);
    if (population.empty())
        return contributions;

    for (const auto& weight : m_weights) {
        double best_val = std::numeric_limits<double>::infinity();
        double second_best_val = std::numeric_limits<double>::infinity();
        size_t best_idx = 0;

        for (size_t p = 0; p < population.size(); ++p) {
            double max_diff = -std::numeric_limits<double>::infinity();
            for (size_t i = 0; i < population[p].objectives.size(); ++i) {
                double diff = weight[i] * std::abs(population[p].objectives[i] - m_ideal_point[i]);
                if (diff > max_diff)
                    max_diff = diff;
            }

            if (max_diff < best_val) {
                second_best_val = best_val;
                best_val = max_diff;
                best_idx = p;
            } else if (max_diff < second_best_val) {
                second_best_val = max_diff;
            }
        }

        if (second_best_val != std::numeric_limits<double>::infinity()) {
            contributions[best_idx] += (second_best_val - best_val);
        }
    }

    return contributions;
}

} // namespace UavMoea::indicators