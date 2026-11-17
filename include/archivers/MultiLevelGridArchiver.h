#pragma once

#include "IArchiver.h"
#include <cstdint>
#include <vector>

namespace UavMoea::archivers {

class MultiLevelGridArchiver : public IArchiver {
public:
    explicit MultiLevelGridArchiver(uint16_t capacity);

    void update(const std::vector<Individual>& offspring) override;
    const std::vector<Individual>& getArchive() const override;

private:
    uint16_t m_capacity;
    std::vector<Individual> m_archive;

    std::vector<int> getBox(const std::vector<double>& obj, int b) const;
    bool boxDominatesOrEqual(const std::vector<int>& boxA, const std::vector<int>& boxB) const;
};

} // namespace UavMoea::archivers