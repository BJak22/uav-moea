#pragma once

#include <vector>

#include "IIndicator.h"

namespace UavMoea::indicators {

class R2Indicator : public IIndicator {
public:
    R2Indicator(std::vector<std::vector<double>> weights, std::vector<double> ideal_point = {});

    void updateState(const std::vector<Individual>& population) override;
    double evaluate(const std::vector<Individual>& population) const override;
    std::vector<double> calculateContributions(const std::vector<Individual>& population) const override;

private:
    std::vector<std::vector<double>> m_weights;
    std::vector<double> m_ideal_point;
};

} // namespace UavMoea::indicators