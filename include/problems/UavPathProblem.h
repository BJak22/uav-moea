#pragma once

#include <cstddef>
#include <memory>
#include <random>
#include <vector>

#include "problems/IProblem.h"
#include "problems/UavInstance.h"

namespace UavMoea::problems {

using UavGenotype = std::vector<int>;

class UavPathProblem : public IProblem {
public:
    UavPathProblem(std::shared_ptr<const UavInstance> instance, size_t num_uavs,
                   unsigned int seed = std::random_device{}());

    void evaluate(Individual& individual) const override;
    Individual generateRandom() const override;

    [[nodiscard]] const UavInstance& getInstance() const noexcept {
        return *m_instance;
    }
    [[nodiscard]] size_t getNumUavs() const noexcept {
        return m_num_uavs;
    }

private:
    struct RateEvent {
        double time;
        double delta_rate;

        bool operator<(const RateEvent& other) const noexcept {
            return time < other.time;
        }
    };

    std::shared_ptr<const UavInstance> m_instance;
    size_t m_num_uavs;
    double m_inv_num_uavs;
    mutable std::mt19937 m_rng;

    mutable std::vector<double> m_path_times_buf;
    mutable std::vector<RateEvent> m_events_buf;
};

} // namespace UavMoea::problems