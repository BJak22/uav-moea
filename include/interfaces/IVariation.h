#pragma once
#include "Individual.h"

namespace UavMoea::interfaces {

class IVariation {
public:
    virtual ~IVariation() = default;
    virtual uint16_t getRequiredParentsCount(uint16_t lambda) const = 0;

    virtual std::vector<Individual> generateOffspring(const std::vector<Individual>& parents) = 0;
};

} // namespace UavMoea::interfaces