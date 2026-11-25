#include <chrono>
#include <exception>
#include <iomanip>
#include <iostream>
#include <string>
#include <utility>
#include <vector>

#include "core/Experiments.h"

int main() {
    try {
        using namespace UavMoea;
        using Clock = std::chrono::steady_clock;

        const auto total_start = Clock::now();

        ExperimentConfig uav_cfg{.num_runs = 30, .mu = 100, .max_evaluations = 50000, .log_interval = 2500};

        const std::vector<std::pair<std::string, std::string>> test_cases = {{"TC-PGI/tcB.json", "results_B_5uav"},
                                                                             {"TC-PGI/tcT.json", "results_T_5uav"},
                                                                             {"TC-SP/tcGL.json", "results_GL_5uav"},
                                                                             {"TC-SP/tcSW.json", "results_SW_5uav"}};

        constexpr size_t num_uavs = 5;

        std::cout << std::fixed << std::setprecision(2);
        for (const auto& [instance_path, output_prefix] : test_cases) {
            const auto case_start = Clock::now();
            runUavBenchmark(instance_path, output_prefix, num_uavs, uav_cfg);
            const auto case_end = Clock::now();

            const double case_sec = std::chrono::duration<double>(case_end - case_start).count();
            std::cout << " -> Elapsed for " << output_prefix << ": " << case_sec << " s\n\n";
        }

        const auto total_end = Clock::now();
        const double total_sec = std::chrono::duration<double>(total_end - total_start).count();

        std::cout << "All data has been safely saved to the disk. Total elapsed time: " << total_sec << " s ("
                  << (total_sec / 60.0) << " min).\n";
        return 0;
    } catch (const std::exception& e) {
        std::cerr << "Critical error: " << e.what() << '\n';
        return 1;
    } catch (...) {
        std::cerr << "Unknown critical error.\n";
        return 1;
    }
}