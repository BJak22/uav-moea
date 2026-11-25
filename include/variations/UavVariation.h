#pragma once

#include <array>
#include <cstddef>
#include <cstdint>
#include <memory>
#include <random>
#include <vector>

#include "problems/IProblem.h"
#include "problems/UavInstance.h"
#include "variations/IVariation.h"

namespace UavMoea::variations {

class UavVariation : public IVariation {
public:
    UavVariation(std::shared_ptr<const problems::UavInstance> instance, const problems::IProblem& problem,
                 size_t num_uavs, double crossover_prob = 0.9, double mutation_prob = 1.0, double learning_rate = 0.1,
                 double min_operator_prob = 0.05, unsigned int seed = std::random_device{}());

    uint16_t getRequiredParentsCount(uint16_t lambda) const override;
    std::vector<Individual> generateOffspring(const std::vector<Individual>& parents) override;

    [[nodiscard]] std::array<double, 3> getOperatorProbabilities() const noexcept {
        return m_op_probs;
    }

private:
    using Genotype = std::vector<int>;
    using RouteList = std::vector<std::vector<int>>;

    Genotype pmxCrossover(const Genotype& p1, const Genotype& p2);

    void decodeRoutes(const Genotype& genotype);
    void encodeRoutes(Genotype& out) const;
    double computeRouteTotalTime(const std::vector<int>& route) const;
    double computeRouteTravelTime(const std::vector<int>& route) const;

    bool mutateWorkloadBalance(Genotype& genotype);
    bool mutateTraversalMinimization(Genotype& genotype);
    bool mutatePrioritySorting(Genotype& genotype);

    void updateOperatorProbabilities(size_t op_idx, double reward);

    std::shared_ptr<const problems::UavInstance> m_instance;
    const problems::IProblem& m_problem;
    size_t m_num_uavs;

    double m_crossover_prob;
    double m_mutation_prob;
    double m_learning_rate;
    double m_min_prob;

    std::array<double, 3> m_op_probs{1.0 / 3.0, 1.0 / 3.0, 1.0 / 3.0};
    std::mt19937 m_rng;

    std::vector<int> m_perm1_buf;
    std::vector<int> m_perm2_buf;
    std::vector<int> m_child_perm_buf;
    std::vector<size_t> m_pos_in_p2_buf;
    std::vector<uint8_t> m_used_buf;
    RouteList m_routes_buf;
    std::vector<size_t> m_candidate_routes_buf;
};

} // namespace UavMoea::variations