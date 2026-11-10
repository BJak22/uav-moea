#pragma once
#include "interfaces/IArchiver.h"
#include <cstdint>
#include <vector>

namespace UavMoea {

class Nsga2Archiver : public interfaces::IArchiver {
public:
    explicit Nsga2Archiver(uint16_t capacity);

    void update(const std::vector<Individual>& offspring) override;
    const std::vector<Individual>& getArchive() const override;

private:
    uint16_t m_capacity;
    std::vector<Individual> m_archive;

    std::vector<std::vector<size_t>> fastNonDominatedSort(const std::vector<Individual>& population) const;
    void calculateCrowdingDistance(std::vector<Individual>& population, const std::vector<size_t>& front) const;
    bool dominates(const Individual& a, const Individual& b) const;
};

} // namespace UavMoea