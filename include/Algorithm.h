#pragma once
#include "AsyncLogger.h"
#include "Individual.h"
#include "interfaces/IArchiver.h"
#include "interfaces/IProblem.h"
#include "interfaces/ISelection.h"
#include "interfaces/IVariation.h"
#include <memory>

namespace UavMoea {

class Algorithm {
public:
    Algorithm(std::unique_ptr<interfaces::IArchiver> archiver, std::unique_ptr<interfaces::IVariation> variation,
              std::unique_ptr<interfaces::IProblem> problem, std::unique_ptr<interfaces::ISelection> selection);
    void run(uint16_t max_evaluations, uint16_t mu, uint16_t lambda, AsyncLogger* logger = nullptr,
             int experiment_id = 0, int log_interval = 100);

private:
    std::unique_ptr<interfaces::IArchiver> m_archiver;
    std::unique_ptr<interfaces::IVariation> m_variation;
    std::unique_ptr<interfaces::IProblem> m_problem;
    std::unique_ptr<interfaces::ISelection> m_selection;
};

} // namespace UavMoea