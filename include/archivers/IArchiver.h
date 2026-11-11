#pragma once

#include "core/Individual.h"

namespace UavMoea::archivers {

class IArchiver {
public:
    virtual ~IArchiver() = default;
    virtual void update(const std::vector<Individual>& offspring) = 0;
    virtual const std::vector<Individual>& getArchive() const = 0;
};

} // namespace UavMoea::archivers