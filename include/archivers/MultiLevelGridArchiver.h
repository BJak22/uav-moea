#pragma once

#include <cstdint>
#include <random>
#include <vector>

#include "IArchiver.h"

namespace UavMoea::archivers {

class MultiLevelGridArchiver : public IArchiver {
public:
    explicit MultiLevelGridArchiver(uint16_t capacity);

    void update(const std::vector<Individual>& offspring) override;
    const std::vector<Individual>& getArchive() const override;

private:
    uint16_t m_capacity;
    std::vector<Individual> m_archive;
    mutable std::mt19937 m_rng;

    std::vector<int64_t> getBox(const std::vector<double>& obj, int b) const;
    bool boxDominatesOrEqual(const std::vector<int64_t>& boxA, const std::vector<int64_t>& boxB) const;
};

} // namespace UavMoea::archivers