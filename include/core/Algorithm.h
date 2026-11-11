#pragma once

#include <memory>

#include "AsyncLogger.h"
#include "Individual.h"
#include "archivers/IArchiver.h"
#include "problems/IProblem.h"
#include "selections/ISelection.h"
#include "variations/IVariation.h"

namespace UavMoea {

class Algorithm {
public:
    Algorithm(std::unique_ptr<archivers::IArchiver> archiver, std::unique_ptr<variations::IVariation> variation,
              std::unique_ptr<problems::IProblem> problem, std::unique_ptr<selections::ISelection> selection);
    void run(uint16_t max_evaluations, uint16_t mu, uint16_t lambda, AsyncLogger* logger = nullptr,
             int experiment_id = 0, int log_interval = 100);

private:
    std::unique_ptr<archivers::IArchiver> m_archiver;
    std::unique_ptr<variations::IVariation> m_variation;
    std::unique_ptr<problems::IProblem> m_problem;
    std::unique_ptr<selections::ISelection> m_selection;
};

} // namespace UavMoea