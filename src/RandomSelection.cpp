#include "RandomSelection.h"

namespace UavMoea {

RandomSelection::RandomSelection() : m_rng(std::random_device{}()) {}

std::vector<Individual> RandomSelection::selectParents(const std::vector<Individual>& archive, uint16_t count) const {
    std::uniform_int_distribution<size_t> dist(0, archive.size() - 1);

    std::vector<Individual> parents;
    for (uint16_t i = 0; i < count; ++i) {
        parents.push_back(archive[dist(m_rng)]);
    }
    return parents;
}
} // namespace UavMoea