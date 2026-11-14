#pragma once

#include <cstdint>
#include <memory>
#include <vector>

#include "archivers/IArchiver.h"
#include "indicators/IIndicator.h"

namespace UavMoea::archivers {

class IndicatorBasedArchiver : public IArchiver {
public:
    IndicatorBasedArchiver(uint16_t capacity, std::unique_ptr<indicators::IIndicator> indicator);

    void update(const std::vector<Individual>& offspring) override;
    const std::vector<Individual>& getArchive() const override;

private:
    uint16_t m_capacity;
    std::vector<Individual> m_archive;
    std::unique_ptr<indicators::IIndicator> m_indicator;
};

} // namespace UavMoea::archivers