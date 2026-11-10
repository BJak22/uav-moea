#pragma once
#include "Individual.h"

namespace UavMoea::interfaces {

class IProblem {
public:
    virtual ~IProblem() = default;
    virtual void evaluate(Individual& individual) const = 0;
    virtual Individual generateRandom() const = 0;
};

} // namespace UavMoea::interfaces