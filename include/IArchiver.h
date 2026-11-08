#pragma once
#include "Individual.h"

namespace UavMoea {

class IArchiver {
public:
    virtual ~IArchiver() = default;
    virtual void update(const std::vector<Individual>& offspring) = 0;
    virtual const std::vector<Individual>& getArchive() const = 0;
};

} // namespace UavMoea