#include "Algorithm.h"
#include "AsyncLogger.h"
#include "Nsga2Archiver.h"
#include "RandomSelection.h"
#include "SbxVariation.h"
#include "Zdt1Problem.h"
#include <future>
#include <iostream>
#include <memory>
#include <vector>

using namespace UavMoea;

void runExperiment(int experiment_id, AsyncLogger& logger) {
    uint16_t mu = 100;
    uint16_t lambda = 100;
    uint16_t max_evaluations = 25000;
    int log_interval = 2500;

    auto problem = std::make_unique<Zdt1Problem>(30);
    auto archiver = std::make_unique<Nsga2Archiver>(mu);
    auto variation = std::make_unique<SbxVariation>();
    auto selection = std::make_unique<RandomSelection>();

    Algorithm optimizer(std::move(archiver), std::move(variation), std::move(problem), std::move(selection));

    optimizer.run(max_evaluations, mu, lambda, &logger, experiment_id, log_interval);
}

int main() {
    int num_experiments = 100;

    {
        AsyncLogger logger("experiments_results.csv");

        std::cout << "Running " << num_experiments << " experiments in parallel..." << std::endl;

        std::vector<std::future<void>> futures;
        futures.reserve(num_experiments);

        for (int i = 0; i < num_experiments; ++i) {
            futures.push_back(std::async(std::launch::async, runExperiment, i, std::ref(logger)));
        }

        for (auto& fut : futures) {
            fut.get();
        }

        std::cout << "All experiments completed. Background saves are being finalized..." << std::endl;
    }

    std::cout << "All data has been safely saved to the disk. End of program." << std::endl;

    return 0;
}