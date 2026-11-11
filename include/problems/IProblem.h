#pragma once

#include "core/Individual.h"

namespace UavMoea::problems {

class IProblem {
public:
    virtual ~IProblem() = default;
    virtual void evaluate(Individual& individual) const = 0;
    virtual Individual generateRandom() const = 0;
};

} // namespace UavMoea::problems