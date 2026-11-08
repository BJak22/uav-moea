#pragma once
#include "IArchiver.h"
#include "IVariation.h"
#include "Individual.h"
#include <memory>

namespace UavMoea {

class MoeaAlgorithm {
public:
    MoeaAlgorithm(std::unique_ptr<IArchiver> archiver, std::unique_ptr<IVariation> variation);

    void run(int max_evaluations);

private:
    std::unique_ptr<IArchiver> m_archiver;
    std::unique_ptr<IVariation> m_variation;
};

} // namespace UavMoea