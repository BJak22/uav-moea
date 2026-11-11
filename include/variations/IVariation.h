#pragma once

#include "core/Individual.h"

namespace UavMoea::variations {

class IVariation {
public:
    virtual ~IVariation() = default;
    virtual uint16_t getRequiredParentsCount(uint16_t lambda) const = 0;

    virtual std::vector<Individual> generateOffspring(const std::vector<Individual>& parents) = 0;
};

} // namespace UavMoea::variations