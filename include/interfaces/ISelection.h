#pragma once
#include "Individual.h"

namespace UavMoea::interfaces {

class ISelection {
public:
    virtual ~ISelection() = default;
    virtual std::vector<Individual> selectParents(const std::vector<Individual>& archive, uint16_t count) const = 0;
};

} // namespace UavMoea::interfaces