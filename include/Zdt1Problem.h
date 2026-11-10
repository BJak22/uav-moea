#pragma once
#include "interfaces/IProblem.h"
#include <random>

namespace UavMoea {

class Zdt1Problem : public interfaces::IProblem {
public:
    explicit Zdt1Problem(int dimensions = 30);

    void evaluate(Individual& individual) const override;
    Individual generateRandom() const override;

private:
    int m_dimensions;
    mutable std::mt19937 m_rng;
};

} // namespace UavMoea