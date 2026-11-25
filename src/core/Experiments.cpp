#include "core/Experiments.h"

#include <algorithm>
#include <filesystem>
#include <future>
#include <iostream>
#include <memory>
#include <stdexcept>

#include "archivers/IndicatorBasedArchiver.h"
#include "archivers/MultiLevelGridArchiver.h"
#include "archivers/Nsga2Archiver.h"
#include "core/Algorithm.h"
#include "core/AsyncLogger.h"
#include "indicators/R2Indicator.h"
#include "problems/UavInstance.h"
#include "problems/UavPathProblem.h"
#include "problems/Zdt1Problem.h"
#include "selections/RandomSelection.h"
#include "variations/SbxVariation.h"
#include "variations/UavVariation.h"

namespace UavMoea {

namespace {

void generateSimplexRecursive(size_t obj_idx, size_t num_objectives, size_t remaining_steps, size_t total_divisions,
                              std::vector<double>& current, std::vector<std::vector<double>>& result) {
    if (obj_idx + 1 == num_objectives) {
        current[obj_idx] = static_cast<double>(remaining_steps) / static_cast<double>(total_divisions);
        result.push_back(current);
        return;
    }

    for (size_t step = 0; step <= remaining_steps; ++step) {
        current[obj_idx] = static_cast<double>(step) / static_cast<double>(total_divisions);
        generateSimplexRecursive(obj_idx + 1, num_objectives, remaining_steps - step, total_divisions, current, result);
    }
}

void executeSingleRun(int experiment_id, AsyncLogger& logger, std::unique_ptr<problems::IProblem> problem,
                      std::unique_ptr<archivers::IArchiver> archiver, std::unique_ptr<variations::IVariation> variation,
                      std::unique_ptr<selections::ISelection> selection, const ExperimentConfig& cfg, uint16_t lambda) {
    try {
        Algorithm optimizer(std::move(archiver), std::move(variation), std::move(problem), std::move(selection));
        optimizer.run(cfg.max_evaluations, cfg.mu, lambda, &logger, experiment_id, cfg.log_interval);
    } catch (const std::exception& e) {
        std::cerr << "Error in experiment nr " << experiment_id << ": " << e.what() << '\n';
    } catch (...) {
        std::cerr << "Unknown error in experiment nr " << experiment_id << '\n';
    }
}

std::filesystem::path resolveInstancePath(const std::string& relative_path) {
    namespace fs = std::filesystem;
    fs::path local = fs::path("data") / relative_path;
    if (fs::exists(local)) {
        return local;
    }
#ifdef UAV_DATA_DIR
    fs::path source_data = fs::path(UAV_DATA_DIR) / relative_path;
    if (fs::exists(source_data)) {
        return source_data;
    }
#endif
    return relative_path;
}

} // namespace

std::vector<std::vector<double>> generateDasDennisWeights(size_t num_objectives, size_t divisions) {
    if (num_objectives == 0 || divisions == 0) {
        throw std::invalid_argument("num_objectives and divisions must be positive");
    }
    std::vector<std::vector<double>> weights;
    std::vector<double> current(num_objectives, 0.0);
    generateSimplexRecursive(0, num_objectives, divisions, divisions, current, weights);

    for (auto& w : weights) {
        for (double& val : w) {
            val = std::max(val, 1e-6);
        }
    }
    return weights;
}

void runZdt1Benchmark(const ExperimentConfig& cfg) {
    const auto r2_weights = generateDasDennisWeights(2, 100);

    AsyncLogger logger_nsga2("zdt1_nsga2.csv");
    AsyncLogger logger_r2("zdt1_r2.csv");
    AsyncLogger logger_mga("zdt1_mga.csv");

    std::cout << "[ZDT1] Running " << cfg.num_runs << " runs per archiver...\n";

    std::vector<std::future<void>> futures;
    futures.reserve(cfg.num_runs * 3);

    auto dispatch = [&](int id, AsyncLogger& log, std::unique_ptr<archivers::IArchiver> arch, uint16_t lambda) {
        auto prob = std::make_unique<problems::Zdt1Problem>(30);
        auto var = std::make_unique<variations::SbxVariation>();
        auto sel = std::make_unique<selections::RandomSelection>();

        futures.push_back(std::async(std::launch::async, executeSingleRun, id, std::ref(log), std::move(prob),
                                     std::move(arch), std::move(var), std::move(sel), std::cref(cfg), lambda));
    };

    for (uint32_t i = 0; i < cfg.num_runs; ++i) {
        const int run_id = static_cast<int>(i);
        dispatch(run_id, logger_nsga2, std::make_unique<archivers::Nsga2Archiver>(cfg.mu), cfg.mu);

        auto r2_ind = std::make_unique<indicators::R2Indicator>(r2_weights);
        dispatch(run_id, logger_r2, std::make_unique<archivers::IndicatorBasedArchiver>(cfg.mu, std::move(r2_ind)), 1);

        dispatch(run_id, logger_mga, std::make_unique<archivers::MultiLevelGridArchiver>(cfg.mu), 1);
    }

    for (auto& fut : futures) {
        fut.get();
    }
    std::cout << "[ZDT1] Completed.\n";
}

void runUavBenchmark(const std::string& instance_path, const std::string& output_prefix, size_t num_uavs,
                     const ExperimentConfig& cfg) {
    const std::string full_path = resolveInstancePath(instance_path).string();
    auto instance = std::make_shared<const problems::UavInstance>(problems::UavInstance::loadFromJson(full_path));

    const auto r2_weights = generateDasDennisWeights(3, 13);

    AsyncLogger logger_nsga2(output_prefix + "_nsga2.csv", 3);
    AsyncLogger logger_r2(output_prefix + "_r2.csv", 3);
    AsyncLogger logger_mga(output_prefix + "_mga.csv", 3);

    std::cout << "[UAV] Instance: " << instance_path << " | Non-zero regions: " << instance->num_regions
              << " | UAVs: " << num_uavs << " | Runs per archiver: " << cfg.num_runs << "...\n";

    std::vector<std::future<void>> futures;
    futures.reserve(cfg.num_runs * 3);

    auto dispatch = [&](int id, AsyncLogger& log, std::unique_ptr<archivers::IArchiver> arch, uint16_t lambda,
                        unsigned int seed) {
        auto prob = std::make_unique<problems::UavPathProblem>(instance, num_uavs, seed);
        auto var =
            std::make_unique<variations::UavVariation>(instance, *prob, num_uavs, 0.9, 1.0, 0.1, 0.05, seed + 100000U);
        auto sel = std::make_unique<selections::RandomSelection>();

        futures.push_back(std::async(std::launch::async, executeSingleRun, id, std::ref(log), std::move(prob),
                                     std::move(arch), std::move(var), std::move(sel), std::cref(cfg), lambda));
    };

    for (uint32_t i = 0; i < cfg.num_runs; ++i) {
        const int run_id = static_cast<int>(i);
        const unsigned int base_seed = 42U + i * 1000U;

        // 1. NSGA-II
        dispatch(run_id, logger_nsga2, std::make_unique<archivers::Nsga2Archiver>(cfg.mu), cfg.mu, base_seed + 1U);

        // 2. R2-EMOA
        auto r2_ind = std::make_unique<indicators::R2Indicator>(r2_weights);
        dispatch(run_id, logger_r2, std::make_unique<archivers::IndicatorBasedArchiver>(cfg.mu, std::move(r2_ind)), 1,
                 base_seed + 2U);

        // 3. MGA
        dispatch(run_id, logger_mga, std::make_unique<archivers::MultiLevelGridArchiver>(cfg.mu), 1, base_seed + 3U);
    }

    for (auto& fut : futures) {
        fut.get();
    }
    std::cout << "[UAV] Completed: " << output_prefix << "\n";
}

} // namespace UavMoea