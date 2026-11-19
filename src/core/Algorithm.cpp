#include "core/Algorithm.h"

#include <iostream>
#include <random>

namespace UavMoea {

Algorithm::Algorithm(std::unique_ptr<archivers::IArchiver> archiver, std::unique_ptr<variations::IVariation> variation,
                     std::unique_ptr<problems::IProblem> problem, std::unique_ptr<selections::ISelection> selection)
    : m_archiver(std::move(archiver)), m_variation(std::move(variation)), m_problem(std::move(problem)),
      m_selection(std::move(selection)) {}

void Algorithm::run(uint32_t max_evaluations, uint16_t mu, uint16_t lambda, AsyncLogger* logger, int experiment_id,
                    uint32_t log_interval) {
    std::vector<Individual> initial_pop;
    initial_pop.reserve(mu);
    for (uint16_t i = 0; i < mu; ++i) {
        Individual ind = m_problem->generateRandom();
        m_problem->evaluate(ind);
        initial_pop.push_back(std::move(ind));
    }
    m_archiver->update(initial_pop);

    uint32_t evaluations = mu;
    uint32_t next_log = log_interval;

    while (evaluations < max_evaluations) {
        const auto& archive = m_archiver->getArchive();

        uint16_t required_parents = m_variation->getRequiredParentsCount(lambda);
        std::vector<Individual> parents = m_selection->selectParents(archive, required_parents);
        std::vector<Individual> offsprings = m_variation->generateOffspring(parents);

        uint32_t remaining = max_evaluations - evaluations;
        if (offsprings.size() > remaining) {
            offsprings.resize(remaining);
        }

        for (auto& offspring : offsprings) {
            m_problem->evaluate(offspring);
            evaluations++;
        }

        m_archiver->update(offsprings);

        if (logger && log_interval > 0 && evaluations >= next_log) {
            std::vector<std::vector<double>> obj_snapshot;
            const auto& current_archive = m_archiver->getArchive();
            obj_snapshot.reserve(current_archive.size());
            for (const auto& ind : current_archive) {
                obj_snapshot.push_back(ind.objectives);
            }
            logger->log(experiment_id, evaluations, obj_snapshot);
            while (next_log <= evaluations) {
                next_log += log_interval;
            }
        }
    }
}

} // namespace UavMoea