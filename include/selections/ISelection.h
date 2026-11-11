#pragma once

#include "core/Individual.h"

namespace UavMoea::selections {

class ISelection {
public:
    virtual ~ISelection() = default;
    virtual std::vector<Individual> selectParents(const std::vector<Individual>& archive, uint16_t count) const = 0;
};

} // namespace UavMoea::selections