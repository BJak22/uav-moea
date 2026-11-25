#include "variations/UavVariation.h"

#include <algorithm>
#include <limits>
#include <numeric>
#include <stdexcept>

namespace UavMoea::variations {

UavVariation::UavVariation(std::shared_ptr<const problems::UavInstance> instance, const problems::IProblem& problem,
                           size_t num_uavs, double crossover_prob, double mutation_prob, double learning_rate,
                           double min_operator_prob, unsigned int seed)
    : m_instance(std::move(instance)), m_problem(problem), m_num_uavs(num_uavs), m_crossover_prob(crossover_prob),
      m_mutation_prob(mutation_prob), m_learning_rate(learning_rate), m_min_prob(min_operator_prob), m_rng(seed) {
    if (!m_instance) {
        throw std::invalid_argument("UavInstance cannot be null");
    }
    if (m_num_uavs == 0) {
        throw std::invalid_argument("num_uavs must be at least 1");
    }

    const size_t n = m_instance->num_regions;
    m_perm1_buf.reserve(n);
    m_perm2_buf.reserve(n);
    m_child_perm_buf.resize(n, 0);
    m_pos_in_p2_buf.resize(n + 1, 0);
    m_used_buf.resize(n + 1, 0);
    m_routes_buf.resize(m_num_uavs);
    for (auto& r : m_routes_buf) {
        r.reserve(n);
    }
    m_candidate_routes_buf.reserve(m_num_uavs);
}

uint16_t UavVariation::getRequiredParentsCount(uint16_t lambda) const {
    return static_cast<uint16_t>(std::max<size_t>(2, ((static_cast<size_t>(lambda) + 1) >> 1) << 1));
}

std::vector<Individual> UavVariation::generateOffspring(const std::vector<Individual>& parents) {
    if (parents.size() < 2) {
        throw std::invalid_argument("UavVariation requires at least 2 parents");
    }

    std::uniform_real_distribution<double> prob_dist(0.0, 1.0);

    std::vector<Individual> offsprings;
    offsprings.reserve(parents.size());

    for (size_t i = 0; i + 1 < parents.size(); i += 2) {
        const auto& g1 = std::any_cast<const Genotype&>(parents[i].genotype);
        const auto& g2 = std::any_cast<const Genotype&>(parents[i + 1].genotype);

        std::array<Genotype, 2> child_genotypes;
        if (prob_dist(m_rng) < m_crossover_prob) {
            child_genotypes[0] = pmxCrossover(g1, g2);
            child_genotypes[1] = pmxCrossover(g2, g1);
        } else {
            child_genotypes[0] = g1;
            child_genotypes[1] = g2;
        }

        for (size_t c = 0; c < 2; ++c) {
            Individual child;
            child.genotype = std::move(child_genotypes[c]);

            if (prob_dist(m_rng) < m_mutation_prob) {
                Individual pre_mut = child;
                m_problem.evaluate(pre_mut);

                const double r = prob_dist(m_rng);
                const size_t op_idx = (r < m_op_probs[0]) ? 0 : ((r < m_op_probs[0] + m_op_probs[1]) ? 1 : 2);
                auto& g_mut = std::any_cast<Genotype&>(child.genotype);

                bool changed = false;
                if (op_idx == 0) {
                    changed = mutateWorkloadBalance(g_mut);
                } else if (op_idx == 1) {
                    changed = mutateTraversalMinimization(g_mut);
                } else {
                    changed = mutatePrioritySorting(g_mut);
                }

                double reward = 0.0;
                if (changed) {
                    m_problem.evaluate(child);
                    if (child.dominates(pre_mut)) {
                        reward = 2.0;
                    } else if (!pre_mut.dominates(child)) {
                        reward = 1.0;
                    }
                } else {
                    child.objectives = std::move(pre_mut.objectives);
                }
                updateOperatorProbabilities(op_idx, reward);
            }

            offsprings.push_back(std::move(child));
        }
    }

    return offsprings;
}

UavVariation::Genotype UavVariation::pmxCrossover(const Genotype& p1, const Genotype& p2) {
    const size_t total_len = p1.size();
    const size_t n = m_instance->num_regions;

    m_perm1_buf.clear();
    m_perm2_buf.clear();

    for (int gene : p1) {
        if (gene != -1) {
            m_perm1_buf.push_back(gene);
        }
    }
    for (int gene : p2) {
        if (gene != -1) {
            m_perm2_buf.push_back(gene);
        }
    }

    std::fill(m_child_perm_buf.begin(), m_child_perm_buf.end(), 0);
    if (n > 1) {
        std::uniform_int_distribution<size_t> cut_dist(0, n - 1);
        size_t c1 = cut_dist(m_rng);
        size_t c2 = cut_dist(m_rng);
        if (c1 > c2) {
            std::swap(c1, c2);
        }

        for (size_t idx = 0; idx < n; ++idx) {
            m_pos_in_p2_buf[static_cast<size_t>(m_perm2_buf[idx])] = idx;
        }

        std::fill(m_used_buf.begin(), m_used_buf.end(), 0);
        for (size_t idx = c1; idx <= c2; ++idx) {
            m_child_perm_buf[idx] = m_perm1_buf[idx];
            m_used_buf[static_cast<size_t>(m_perm1_buf[idx])] = 1;
        }

        for (size_t idx = c1; idx <= c2; ++idx) {
            const int val_p2 = m_perm2_buf[idx];
            if (!m_used_buf[static_cast<size_t>(val_p2)]) {
                size_t target_idx = idx;
                while (target_idx >= c1 && target_idx <= c2) {
                    const int mapped_val = m_perm1_buf[target_idx];
                    target_idx = m_pos_in_p2_buf[static_cast<size_t>(mapped_val)];
                }
                m_child_perm_buf[target_idx] = val_p2;
                m_used_buf[static_cast<size_t>(val_p2)] = 1;
            }
        }

        for (size_t idx = 0; idx < n; ++idx) {
            if (m_child_perm_buf[idx] == 0) {
                m_child_perm_buf[idx] = m_perm2_buf[idx];
            }
        }
    } else if (n == 1) {
        m_child_perm_buf[0] = m_perm1_buf[0];
    }

    Genotype child(total_len, -1);
    size_t perm_cursor = 0;
    for (size_t idx = 0; idx < total_len; ++idx) {
        if (p1[idx] != -1) {
            child[idx] = m_child_perm_buf[perm_cursor++];
        }
    }

    return child;
}

void UavVariation::decodeRoutes(const Genotype& genotype) {
    for (auto& route : m_routes_buf) {
        route.clear();
    }
    size_t uav_idx = 0;
    for (int gene : genotype) {
        if (gene == -1) {
            ++uav_idx;
        } else if (uav_idx < m_num_uavs) {
            m_routes_buf[uav_idx].push_back(gene);
        }
    }
}

void UavVariation::encodeRoutes(Genotype& out) const {
    out.clear();
    for (size_t i = 0; i < m_routes_buf.size(); ++i) {
        if (i > 0) {
            out.push_back(-1);
        }
        out.insert(out.end(), m_routes_buf[i].begin(), m_routes_buf[i].end());
    }
}

double UavVariation::computeRouteTotalTime(const std::vector<int>& route) const {
    if (route.empty()) {
        return 0.0;
    }
    double total = 0.0;
    size_t prev = 0;
    for (int region_id : route) {
        const auto r = static_cast<size_t>(region_id);
        total += m_instance->getTravelTime(prev, r) + m_instance->scan_time[r];
        prev = r;
    }
    total += m_instance->getTravelTime(prev, 0);
    return total;
}

double UavVariation::computeRouteTravelTime(const std::vector<int>& route) const {
    if (route.empty()) {
        return 0.0;
    }
    double travel = 0.0;
    size_t prev = 0;
    for (int region_id : route) {
        const auto r = static_cast<size_t>(region_id);
        travel += m_instance->getTravelTime(prev, r);
        prev = r;
    }
    travel += m_instance->getTravelTime(prev, 0);
    return travel;
}

bool UavVariation::mutateWorkloadBalance(Genotype& genotype) {
    if (m_num_uavs < 2) {
        return false;
    }

    decodeRoutes(genotype);
    size_t max_idx = 0;
    size_t min_idx = 0;
    double max_time = -1.0;
    double min_time = std::numeric_limits<double>::infinity();

    for (size_t i = 0; i < m_num_uavs; ++i) {
        const double t = computeRouteTotalTime(m_routes_buf[i]);
        if (t > max_time) {
            max_time = t;
            max_idx = i;
        }
        if (t < min_time) {
            min_time = t;
            min_idx = i;
        }
    }

    if (max_idx == min_idx || m_routes_buf[max_idx].empty()) {
        return false;
    }

    std::uniform_int_distribution<size_t> src_dist(0, m_routes_buf[max_idx].size() - 1);
    const size_t pick_pos = src_dist(m_rng);
    const int moved_region = m_routes_buf[max_idx][pick_pos];
    m_routes_buf[max_idx].erase(m_routes_buf[max_idx].begin() + static_cast<std::ptrdiff_t>(pick_pos));

    std::uniform_int_distribution<size_t> dst_dist(0, m_routes_buf[min_idx].size());
    const size_t insert_pos = dst_dist(m_rng);
    m_routes_buf[min_idx].insert(m_routes_buf[min_idx].begin() + static_cast<std::ptrdiff_t>(insert_pos), moved_region);

    encodeRoutes(genotype);
    return true;
}

bool UavVariation::mutateTraversalMinimization(Genotype& genotype) {
    decodeRoutes(genotype);

    size_t target_idx = 0;
    double max_travel = -1.0;
    for (size_t i = 0; i < m_num_uavs; ++i) {
        const double tr = computeRouteTravelTime(m_routes_buf[i]);
        if (tr > max_travel) {
            max_travel = tr;
            target_idx = i;
        }
    }

    auto& route = m_routes_buf[target_idx];
    if (route.size() < 2) {
        return false;
    }

    std::uniform_int_distribution<size_t> pick_dist(0, route.size() - 1);
    const size_t orig_pos = pick_dist(m_rng);
    const int region = route[orig_pos];
    const auto r_node = static_cast<size_t>(region);

    const size_t orig_prev = (orig_pos == 0) ? 0 : static_cast<size_t>(route[orig_pos - 1]);
    const size_t orig_next = (orig_pos + 1 == route.size()) ? 0 : static_cast<size_t>(route[orig_pos + 1]);
    const double orig_local_cost = m_instance->getTravelTime(orig_prev, r_node) +
                                   m_instance->getTravelTime(r_node, orig_next) -
                                   m_instance->getTravelTime(orig_prev, orig_next);

    route.erase(route.begin() + static_cast<std::ptrdiff_t>(orig_pos));

    for (size_t pos = 0; pos <= route.size(); ++pos) {
        if (pos == orig_pos) {
            continue;
        }
        const size_t prev = (pos == 0) ? 0 : static_cast<size_t>(route[pos - 1]);
        const size_t next = (pos == route.size()) ? 0 : static_cast<size_t>(route[pos]);
        const double new_local_cost = m_instance->getTravelTime(prev, r_node) +
                                      m_instance->getTravelTime(r_node, next) - m_instance->getTravelTime(prev, next);

        if (new_local_cost + 1e-9 < orig_local_cost) {
            route.insert(route.begin() + static_cast<std::ptrdiff_t>(pos), region);
            encodeRoutes(genotype);
            return true;
        }
    }

    return false;
}

bool UavVariation::mutatePrioritySorting(Genotype& genotype) {
    decodeRoutes(genotype);

    m_candidate_routes_buf.clear();
    for (size_t i = 0; i < m_num_uavs; ++i) {
        if (m_routes_buf[i].size() >= 2) {
            m_candidate_routes_buf.push_back(i);
        }
    }
    if (m_candidate_routes_buf.empty()) {
        return false;
    }

    std::uniform_int_distribution<size_t> route_dist(0, m_candidate_routes_buf.size() - 1);
    auto& route = m_routes_buf[m_candidate_routes_buf[route_dist(m_rng)]];

    std::uniform_int_distribution<size_t> idx_dist(0, route.size() - 1);
    size_t start = idx_dist(m_rng);
    size_t end = idx_dist(m_rng);
    if (start > end) {
        std::swap(start, end);
    }
    if (start == end) {
        if (end + 1 < route.size()) {
            ++end;
        } else {
            --start;
        }
    }

    const auto begin_it = route.begin() + static_cast<std::ptrdiff_t>(start);
    const auto end_it = route.begin() + static_cast<std::ptrdiff_t>(end + 1);

    const auto density_cmp = [this](int a, int b) {
        return m_instance->detection_rate[static_cast<size_t>(a)] > m_instance->detection_rate[static_cast<size_t>(b)];
    };

    if (std::is_sorted(begin_it, end_it, density_cmp)) {
        return false;
    }

    std::sort(begin_it, end_it, density_cmp);
    encodeRoutes(genotype);
    return true;
}

void UavVariation::updateOperatorProbabilities(size_t op_idx, double reward) {
    for (size_t i = 0; i < 3; ++i) {
        const double target = (i == op_idx && reward > 0.0) ? (reward * 0.5) : 0.0;
        m_op_probs[i] = (1.0 - m_learning_rate) * m_op_probs[i] + m_learning_rate * target;
    }

    const double free_mass = 1.0 - 3.0 * m_min_prob;
    double raw_sum = 0.0;
    for (double& p : m_op_probs) {
        p = std::max(0.0, p - m_min_prob);
        raw_sum += p;
    }

    if (raw_sum > 0.0) {
        const double inv_raw_sum = free_mass / raw_sum;
        for (double& p : m_op_probs) {
            p = m_min_prob + p * inv_raw_sum;
        }
    } else {
        m_op_probs = {1.0 / 3.0, 1.0 / 3.0, 1.0 / 3.0};
    }
}

} // namespace UavMoea::variations