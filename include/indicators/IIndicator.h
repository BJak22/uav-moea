#pragma once

#include <vector>

#include "core/Individual.h"

namespace UavMoea::indicators {

class IIndicator {
public:
    virtual ~IIndicator() = default;

    virtual void updateState(const std::vector<Individual>&) {}
    virtual double evaluate(const std::vector<Individual>& population) const = 0;
    virtual std::vector<double> calculateContributions(const std::vector<Individual>& population) const = 0;
};

} // namespace UavMoea::indicators