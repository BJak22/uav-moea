#include "indicators/EpsilonIndicator.h"

#include <algorithm>
#include <limits>

namespace UavMoea::indicators {

EpsilonIndicator::EpsilonIndicator(std::vector<std::vector<double>> reference_points)
    : m_reference_points(std::move(reference_points)) {}

double EpsilonIndicator::calculateEpsilon(const std::vector<double>& z, const std::vector<double>& a) const {
    double max_eps = -std::numeric_limits<double>::infinity();
    for (size_t i = 0; i < z.size(); ++i) {
        double z_val = (z[i] == 0.0) ? 1e-6 : z[i];

        double eps = (a[i] / z_val) - 1.0;
        if (eps > max_eps) {
            max_eps = eps;
        }
    }
    return max_eps;
}

double EpsilonIndicator::evaluate(const std::vector<Individual>& population) const {
    if (population.empty() || m_reference_points.empty()) {
        return std::numeric_limits<double>::infinity();
    }

    double indicator_value = -std::numeric_limits<double>::infinity();

    for (const auto& z : m_reference_points) {
        double min_eps = std::numeric_limits<double>::infinity();

        for (const auto& ind : population) {
            double eps = calculateEpsilon(z, ind.objectives);
            if (eps < min_eps) {
                min_eps = eps;
            }
        }

        if (min_eps > indicator_value) {
            indicator_value = min_eps;
        }
    }

    return indicator_value;
}

std::vector<double> EpsilonIndicator::calculateContributions(const std::vector<Individual>& population) const {
    std::vector<double> contributions(population.size(), 0.0);
    if (population.empty() || m_reference_points.empty())
        return contributions;

    std::vector<double> d1(m_reference_points.size(), std::numeric_limits<double>::infinity());
    std::vector<double> d2(m_reference_points.size(), std::numeric_limits<double>::infinity());
    std::vector<size_t> best_idx(m_reference_points.size(), 0);

    double base_indicator = -std::numeric_limits<double>::infinity();

    for (size_t j = 0; j < m_reference_points.size(); ++j) {
        for (size_t p = 0; p < population.size(); ++p) {
            double eps = calculateEpsilon(m_reference_points[j], population[p].objectives);
            if (eps < d1[j]) {
                d2[j] = d1[j];
                d1[j] = eps;
                best_idx[j] = p;
            } else if (eps < d2[j]) {
                d2[j] = eps;
            }
        }
        if (d1[j] > base_indicator) {
            base_indicator = d1[j];
        }
    }

    for (size_t p = 0; p < population.size(); ++p) {
        double new_indicator = -std::numeric_limits<double>::infinity();

        for (size_t j = 0; j < m_reference_points.size(); ++j) {
            double current_eps = (best_idx[j] == p) ? d2[j] : d1[j];
            if (current_eps > new_indicator) {
                new_indicator = current_eps;
            }
        }

        if (new_indicator != std::numeric_limits<double>::infinity()) {
            contributions[p] = new_indicator - base_indicator;
        }
    }

    return contributions;
}

} // namespace UavMoea::indicators