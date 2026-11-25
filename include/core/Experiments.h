#pragma once

#include <cstddef>
#include <cstdint>
#include <string>
#include <vector>

namespace UavMoea {

struct ExperimentConfig {
    uint32_t num_runs = 30;
    uint16_t mu = 100;
    uint32_t max_evaluations = 50000;
    uint32_t log_interval = 2500;
};

std::vector<std::vector<double>> generateDasDennisWeights(size_t num_objectives, size_t divisions);

void runZdt1Benchmark(const ExperimentConfig& cfg = {});

void runUavBenchmark(const std::string& instance_path, const std::string& output_prefix, size_t num_uavs,
                     const ExperimentConfig& cfg = {});

} // namespace UavMoea