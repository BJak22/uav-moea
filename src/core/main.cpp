#include <future>
#include <iostream>
#include <memory>
#include <vector>

#include "archivers/IndicatorBasedArchiver.h"
#include "archivers/Nsga2Archiver.h"
#include "core/Algorithm.h"
#include "core/AsyncLogger.h"
#include "indicators/R2Indicator.h"
#include "problems/Zdt1Problem.h"
#include "selections/RandomSelection.h"
#include "variations/SbxVariation.h"

using namespace UavMoea;

void runExperiment(int experiment_id, AsyncLogger& logger, std::unique_ptr<problems::IProblem> problem,
                   std::unique_ptr<archivers::IArchiver> archiver, std::unique_ptr<variations::IVariation> variation,
                   std::unique_ptr<selections::ISelection> selection, uint16_t capacity, uint16_t lambda) {

    uint16_t max_evaluations = 25000;
    int log_interval = 2500;

    Algorithm optimizer(std::move(archiver), std::move(variation), std::move(problem), std::move(selection));
    optimizer.run(max_evaluations, capacity, lambda, &logger, experiment_id, log_interval);
}

int main() {
    int num_experiments = 100;
    uint16_t mu = 100;

    std::vector<std::vector<double>> r2_weights;
    for (int w = 0; w <= 100; ++w) {
        double w1 = w / 100.0;
        double w2 = 1.0 - w1;
        r2_weights.push_back({w1, w2});
    }

    {
        AsyncLogger logger_nsga2("experiments_results_nsga2.csv");
        AsyncLogger logger_indicator_based("experiments_results_indicatorBased.csv");

        std::cout << "Running " << num_experiments << " experiments for each archiver in parallel..." << std::endl;

        std::vector<std::future<void>> futures;
        futures.reserve(num_experiments * 2);

        for (int i = 0; i < num_experiments; ++i) {

            auto prob_nsga2 = std::make_unique<problems::Zdt1Problem>(30);
            auto var_nsga2 = std::make_unique<variations::SbxVariation>();
            auto sel_nsga2 = std::make_unique<selections::RandomSelection>();
            auto arch_nsga2 = std::make_unique<archivers::Nsga2Archiver>(mu);

            futures.push_back(std::async(std::launch::async, runExperiment, i, std::ref(logger_nsga2),
                                         std::move(prob_nsga2), std::move(arch_nsga2), std::move(var_nsga2),
                                         std::move(sel_nsga2), mu, uint16_t(100)));

            auto prob_r2 = std::make_unique<problems::Zdt1Problem>(30);
            auto var_r2 = std::make_unique<variations::SbxVariation>();
            auto sel_r2 = std::make_unique<selections::RandomSelection>();

            auto r2_indicator = std::make_unique<indicators::R2Indicator>(r2_weights);
            auto arch_r2 = std::make_unique<archivers::IndicatorBasedArchiver>(mu, std::move(r2_indicator));

            futures.push_back(std::async(std::launch::async, runExperiment, i, std::ref(logger_indicator_based),
                                         std::move(prob_r2), std::move(arch_r2), std::move(var_r2), std::move(sel_r2),
                                         mu, uint16_t(1)));
        }

        for (auto& fut : futures) {
            fut.get();
        }

        std::cout << "All experiments completed. Background saves are being finalized..." << std::endl;
    }

    std::cout << "All data has been safely saved to the disk. End of program." << std::endl;

    return 0;
}