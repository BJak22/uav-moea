#pragma once

#include "IIndicator.h"
#include <stdexcept>
#include <vector>

namespace UavMoea::indicators {

class HypervolumeIndicator : public IIndicator {
public:
    explicit HypervolumeIndicator(std::vector<double> reference_point);

    void updateState(const std::vector<Individual>&) override {}

    double evaluate(const std::vector<Individual>& population) const override;
    std::vector<double> calculateContributions(const std::vector<Individual>& population) const override;

private:
    std::vector<double> m_reference_point;

    struct Point2D {
        double f1;
        double f2;
        size_t original_index;
    };

    std::vector<Point2D> getSortedNonDominated2D(const std::vector<Individual>& population) const;
};

} // namespace UavMoea::indicators