#include "indicators/IgdPlusIndicator.h"

#include <algorithm>
#include <cmath>
#include <limits>

namespace UavMoea::indicators {

IgdPlusIndicator::IgdPlusIndicator(std::vector<std::vector<double>> reference_points)
    : m_reference_points(std::move(reference_points)) {}

double IgdPlusIndicator::calculateModifiedDistance(const std::vector<double>& z, const std::vector<double>& a) const {
    double dist_sq = 0.0;
    for (size_t i = 0; i < z.size(); ++i) {
        double diff = std::max(a[i] - z[i], 0.0);
        dist_sq += diff * diff;
    }
    return std::sqrt(dist_sq);
}

double IgdPlusIndicator::evaluate(const std::vector<Individual>& population) const {
    if (population.empty() || m_reference_points.empty()) {
        return std::numeric_limits<double>::infinity();
    }

    double total_igd_plus = 0.0;

    for (const auto& z : m_reference_points) {
        double min_dist = std::numeric_limits<double>::infinity();

        for (const auto& ind : population) {
            double dist = calculateModifiedDistance(z, ind.objectives);
            if (dist < min_dist) {
                min_dist = dist;
            }
        }
        total_igd_plus += min_dist;
    }

    return total_igd_plus / m_reference_points.size();
}

std::vector<double> IgdPlusIndicator::calculateContributions(const std::vector<Individual>& population) const {
    std::vector<double> contributions(population.size(), 0.0);
    if (population.empty() || m_reference_points.empty()) {
        return contributions;
    }

    for (const auto& z : m_reference_points) {
        double best_dist = std::numeric_limits<double>::infinity();
        double second_best_dist = std::numeric_limits<double>::infinity();
        size_t best_idx = 0;

        for (size_t p = 0; p < population.size(); ++p) {
            double dist = calculateModifiedDistance(z, population[p].objectives);

            if (dist < best_dist) {
                second_best_dist = best_dist;
                best_dist = dist;
                best_idx = p;
            } else if (dist < second_best_dist) {
                second_best_dist = dist;
            }
        }

        if (second_best_dist != std::numeric_limits<double>::infinity()) {
            contributions[best_idx] += (second_best_dist - best_dist);
        }
    }

    return contributions;
}

} // namespace UavMoea::indicators