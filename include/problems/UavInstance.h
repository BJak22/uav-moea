#pragma once

#include <cstddef>
#include <string>
#include <vector>

namespace UavMoea::problems {

struct UavInstance {
    std::string name;
    size_t num_regions = 0;
    double total_population = 0.0;

    std::vector<double> scan_time;
    std::vector<double> population;
    std::vector<double> detection_rate;
    std::vector<double> travel_time;

    [[nodiscard]] inline double getTravelTime(size_t from, size_t to) const noexcept {
        return travel_time[from * (num_regions + 1) + to];
    }

    static UavInstance loadFromJson(const std::string& filepath);
};

} // namespace UavMoea::problems