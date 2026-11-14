#pragma once

#include <vector>

#include "IIndicator.h"

namespace UavMoea::indicators {

class IgdPlusIndicator : public IIndicator {
public:
    explicit IgdPlusIndicator(std::vector<std::vector<double>> reference_points);

    void updateState(const std::vector<Individual>&) override {}

    double evaluate(const std::vector<Individual>& population) const override;
    std::vector<double> calculateContributions(const std::vector<Individual>& population) const override;

private:
    std::vector<std::vector<double>> m_reference_points;
    double calculateModifiedDistance(const std::vector<double>& z, const std::vector<double>& a) const;
};

} // namespace UavMoea::indicators