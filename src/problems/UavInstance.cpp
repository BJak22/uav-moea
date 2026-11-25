#include "problems/UavInstance.h"

#include <fstream>
#include <nlohmann/json.hpp>
#include <stdexcept>

namespace UavMoea::problems {

UavInstance UavInstance::loadFromJson(const std::string& filepath) {
    std::ifstream file(filepath);
    if (!file.is_open()) {
        throw std::runtime_error("Cannot open UAV instance JSON file: " + filepath);
    }

    nlohmann::json j;
    file >> j;

    const size_t raw_count = j.at("node-count").get<size_t>();
    const auto raw_pop = j.at("node-population").get<std::vector<double>>();
    const auto raw_node_time = j.at("time-process-node").get<std::vector<double>>();
    const auto& raw_edge_time = j.contains("time-process_edge")
                                    ? j.at("time-process_edge").get<std::vector<std::vector<double>>>()
                                    : j.at("time-process-edge").get<std::vector<std::vector<double>>>();

    if (raw_count == 0 || raw_pop.size() != raw_count || raw_node_time.size() != raw_count ||
        raw_edge_time.size() != raw_count) {
        throw std::runtime_error("Invalid dimensions in UAV JSON file: " + filepath);
    }

    const bool first_node_is_depot = (raw_pop[0] == 0.0);
    const size_t start_scan_idx = first_node_is_depot ? 1 : 0;

    std::vector<size_t> internal_to_raw;
    internal_to_raw.reserve(raw_count + 1);
    internal_to_raw.push_back(0);

    double total_pop = 0.0;
    for (size_t i = start_scan_idx; i < raw_count; ++i) {
        if (raw_pop[i] > 0.0) {
            internal_to_raw.push_back(i);
            total_pop += raw_pop[i];
        }
    }

    UavInstance inst;
    inst.name = filepath;
    inst.num_regions = internal_to_raw.size() - 1;
    inst.total_population = total_pop;

    const size_t dim = inst.num_regions + 1;
    inst.scan_time.resize(dim, 0.0);
    inst.population.resize(dim, 0.0);
    inst.detection_rate.resize(dim, 0.0);
    inst.travel_time.resize(dim * dim, 0.0);

    for (size_t u = 1; u < dim; ++u) {
        const size_t raw_u = internal_to_raw[u];
        inst.scan_time[u] = raw_node_time[raw_u];
        inst.population[u] = raw_pop[raw_u];
        inst.detection_rate[u] = (inst.scan_time[u] > 0.0) ? (inst.population[u] / inst.scan_time[u]) : 0.0;
    }

    for (size_t u = 0; u < dim; ++u) {
        const size_t raw_u = internal_to_raw[u];
        for (size_t v = 0; v < dim; ++v) {
            const size_t raw_v = internal_to_raw[v];
            if (u == v) {
                inst.travel_time[u * dim + v] = 0.0;
            } else {
                inst.travel_time[u * dim + v] = raw_edge_time[raw_u][raw_v];
            }
        }
    }

    return inst;
}

} // namespace UavMoea::problems