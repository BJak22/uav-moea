#pragma once

#include <random>

#include "IProblem.h"

namespace UavMoea::problems {

class Zdt1Problem : public IProblem {
public:
    explicit Zdt1Problem(int dimensions = 30);

    void evaluate(Individual& individual) const override;
    Individual generateRandom() const override;

private:
    int m_dimensions;
    mutable std::mt19937 m_rng;
};

} // namespace UavMoea::problems