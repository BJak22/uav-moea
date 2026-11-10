#include "Algorithm.h"
#include <iostream>
#include <random>

namespace UavMoea {

Algorithm::Algorithm(std::unique_ptr<interfaces::IArchiver> archiver, std::unique_ptr<interfaces::IVariation> variation,
                     std::unique_ptr<interfaces::IProblem> problem, std::unique_ptr<interfaces::ISelection> selection)
    : m_archiver(std::move(archiver)), m_variation(std::move(variation)), m_problem(std::move(problem)),
      m_selection(std::move(selection)) {}

void Algorithm::run(uint16_t max_evaluations, uint16_t mu, uint16_t lambda, AsyncLogger* logger, int experiment_id,
                    int log_interval) {
    std::vector<Individual> initial_pop;
    for (uint16_t i = 0; i < mu; ++i) {
        Individual ind = m_problem->generateRandom();
        m_problem->evaluate(ind);
        initial_pop.push_back(ind);
    }
    m_archiver->update(initial_pop);
    int evaluations = mu;

    while (evaluations < max_evaluations) {
        const auto& archive = m_archiver->getArchive();

        uint16_t required_parents = m_variation->getRequiredParentsCount(lambda);
        std::vector<Individual> parents = m_selection->selectParents(archive, required_parents);
        std::vector<Individual> offsprings = m_variation->generateOffspring(parents);

        for (auto& offspring : offsprings) {
            m_problem->evaluate(offspring);
            evaluations++;

            if (logger && (evaluations % log_interval == 0)) {
                std::vector<std::vector<double>> obj_snapshot;
                const auto& current_archive = m_archiver->getArchive();
                obj_snapshot.reserve(current_archive.size());
                for (const auto& ind : current_archive) {
                    obj_snapshot.push_back(ind.objectives);
                }
                logger->log(experiment_id, evaluations, obj_snapshot);
            }

            if (evaluations >= max_evaluations)
                break;
        }

        m_archiver->update(offsprings);
    }
}

} // namespace UavMoea