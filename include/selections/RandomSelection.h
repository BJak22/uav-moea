#pragma once

#include <random>

#include "ISelection.h"

namespace UavMoea::selections {

class RandomSelection : public ISelection {
public:
    RandomSelection();
    std::vector<Individual> selectParents(const std::vector<Individual>& archive, uint16_t count) const override;

private:
    mutable std::mt19937 m_rng;
};

} // namespace UavMoea::selections