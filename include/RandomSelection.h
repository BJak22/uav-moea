#pragma once
#include "interfaces/ISelection.h"
#include <random>

namespace UavMoea {

class RandomSelection : public interfaces::ISelection {
public:
    RandomSelection();
    std::vector<Individual> selectParents(const std::vector<Individual>& archive, uint16_t count) const override;

private:
    mutable std::mt19937 m_rng;
};

} // namespace UavMoea