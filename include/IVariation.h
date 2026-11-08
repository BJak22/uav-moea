#pragma once
#include "Individual.h"

namespace UavMoea {

class IVariation {
public:
    virtual ~IVariation() = default;
    virtual std::vector<Individual> generateOffspring(const std::vector<Individual>& parents) = 0;
};

} // namespace UavMoea