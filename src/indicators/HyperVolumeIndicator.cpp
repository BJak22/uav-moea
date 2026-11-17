#include "indicators/HypervolumeIndicator.h"
#include "utils/Pareto.h"
#include <algorithm>

namespace UavMoea::indicators {

HypervolumeIndicator::HypervolumeIndicator(std::vector<double> reference_point)
    : m_reference_point(std::move(reference_point)) {
    if (m_reference_point.size() != 2) {
        throw std::invalid_argument("In this version, HypervolumeIndicator is intended solely for 2 objectives (2D).");
    }
}

std::vector<HypervolumeIndicator::Point2D>
HypervolumeIndicator::getSortedNonDominated2D(const std::vector<Individual>& population) const {

    std::vector<Point2D> valid_points;
    if (population.empty())
        return valid_points;

    std::vector<std::vector<size_t>> fronts = utils::fastNonDominatedSort(population);

    for (size_t idx : fronts[0]) {
        double f1 = population[idx].objectives[0];
        double f2 = population[idx].objectives[1];

        if (f1 < m_reference_point[0] && f2 < m_reference_point[1]) {
            valid_points.push_back({f1, f2, idx});
        }
    }

    std::sort(valid_points.begin(), valid_points.end(), [](const Point2D& a, const Point2D& b) { return a.f1 < b.f1; });

    return valid_points;
}

double HypervolumeIndicator::evaluate(const std::vector<Individual>& population) const {
    auto pts = getSortedNonDominated2D(population);
    if (pts.empty())
        return 0.0;

    double volume = 0.0;

    for (size_t i = 0; i < pts.size(); ++i) {
        double width = m_reference_point[0] - pts[i].f1;
        double prev_f2 = (i == 0) ? m_reference_point[1] : pts[i - 1].f2;
        double height = prev_f2 - pts[i].f2;

        volume += width * height;
    }

    return volume;
}

std::vector<double> HypervolumeIndicator::calculateContributions(const std::vector<Individual>& population) const {
    std::vector<double> contributions(population.size(), 0.0);
    auto pts = getSortedNonDominated2D(population);

    if (pts.empty())
        return contributions;

    if (pts.size() == 1) {
        contributions[pts[0].original_index] = evaluate(population);
        return contributions;
    }

    for (size_t i = 0; i < pts.size(); ++i) {
        double top = (i == 0) ? m_reference_point[1] : pts[i - 1].f2;
        double right = (i == pts.size() - 1) ? m_reference_point[0] : pts[i + 1].f1;
        double width = right - pts[i].f1;
        double height = top - pts[i].f2;

        contributions[pts[i].original_index] = width * height;
    }

    return contributions;
}

} // namespace UavMoea::indicators