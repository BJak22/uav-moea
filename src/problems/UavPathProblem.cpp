#include "problems/UavPathProblem.h"

#include <algorithm>
#include <cmath>
#include <limits>
#include <numeric>
#include <stdexcept>

namespace UavMoea::problems {

UavPathProblem::UavPathProblem(std::shared_ptr<const UavInstance> instance, size_t num_uavs, unsigned int seed)
    : m_instance(std::move(instance)), m_num_uavs(num_uavs),
      m_inv_num_uavs(num_uavs > 0 ? 1.0 / static_cast<double>(num_uavs) : 0.0), m_rng(seed) {
    if (!m_instance) {
        throw std::invalid_argument("UavInstance cannot be null");
    }
    if (m_num_uavs == 0) {
        throw std::invalid_argument("num_uavs must be at least 1");
    }
    m_path_times_buf.resize(m_num_uavs, 0.0);
    m_events_buf.reserve(m_instance->num_regions * 2);
}

Individual UavPathProblem::generateRandom() const {
    const size_t n = m_instance->num_regions;
    const size_t num_separators = m_num_uavs - 1;

    UavGenotype genotype;
    genotype.reserve(n + num_separators);

    for (size_t i = 1; i <= n; ++i) {
        genotype.push_back(static_cast<int>(i));
    }
    for (size_t s = 0; s < num_separators; ++s) {
        genotype.push_back(-1);
    }

    std::shuffle(genotype.begin(), genotype.end(), m_rng);

    Individual ind;
    ind.genotype = std::move(genotype);
    evaluate(ind);
    return ind;
}

void UavPathProblem::evaluate(Individual& individual) const {
    const auto& genotype = std::any_cast<const UavGenotype&>(individual.genotype);

    std::fill(m_path_times_buf.begin(), m_path_times_buf.end(), 0.0);
    m_events_buf.clear();

    size_t uav_idx = 0;
    size_t prev_node = 0;
    double current_time = 0.0;
    bool has_visited_any = false;

    for (int gene : genotype) {
        if (gene == -1) {
            if (has_visited_any) {
                current_time += m_instance->getTravelTime(prev_node, 0);
            }
            m_path_times_buf[uav_idx] = current_time;
            ++uav_idx;
            prev_node = 0;
            current_time = 0.0;
            has_visited_any = false;
        } else {
            const auto region = static_cast<size_t>(gene);
            current_time += m_instance->getTravelTime(prev_node, region);

            const double scan_duration = m_instance->scan_time[region];
            const double rate = m_instance->detection_rate[region];

            if (rate > 0.0) {
                m_events_buf.push_back({current_time, rate});
                m_events_buf.push_back({current_time + scan_duration, -rate});
            }

            current_time += scan_duration;
            prev_node = region;
            has_visited_any = true;
        }
    }

    if (uav_idx < m_num_uavs) {
        if (has_visited_any) {
            current_time += m_instance->getTravelTime(prev_node, 0);
        }
        m_path_times_buf[uav_idx] = current_time;
    }

    const auto [min_it, max_it] = std::minmax_element(m_path_times_buf.begin(), m_path_times_buf.end());
    const double t_min = *min_it;
    const double t_max = *max_it;
    const double f1 = t_max;

    double f2 = std::numeric_limits<double>::infinity();
    if (t_min > 0.0) {
        std::sort(m_events_buf.begin(), m_events_buf.end());

        double area_under_v = 0.0;
        double current_v = 0.0;
        double current_rate = 0.0;
        double last_t = 0.0;

        for (const auto& ev : m_events_buf) {
            if (ev.time >= t_min) {
                break;
            }
            const double dt = ev.time - last_t;
            if (dt > 0.0) {
                const double next_v = current_v + current_rate * dt;
                area_under_v += 0.5 * (current_v + next_v) * dt;
                current_v = next_v;
                last_t = ev.time;
            }
            current_rate += ev.delta_rate;
        }

        if (t_min > last_t) {
            const double dt = t_min - last_t;
            const double next_v = current_v + current_rate * dt;
            area_under_v += 0.5 * (current_v + next_v) * dt;
        }

        if (area_under_v > 0.0) {
            const double total_rect = m_instance->total_population * t_min;
            const double area_over_v = std::max(0.0, total_rect - area_under_v);
            f2 = area_over_v / area_under_v;
        }
    }

    const double sum_t = std::accumulate(m_path_times_buf.begin(), m_path_times_buf.end(), 0.0);
    const double t_mean = sum_t * m_inv_num_uavs;
    double sq_diff_sum = 0.0;
    for (double t : m_path_times_buf) {
        const double diff = t - t_mean;
        sq_diff_sum += diff * diff;
    }
    const double f3 = std::sqrt(sq_diff_sum * m_inv_num_uavs);

    if (individual.objectives.size() == 3) {
        individual.objectives[0] = f1;
        individual.objectives[1] = f2;
        individual.objectives[2] = f3;
    } else {
        individual.objectives = {f1, f2, f3};
    }
}

} // namespace UavMoea::problems