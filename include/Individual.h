#pragma once
#include <any>
#include <vector>

namespace UavMoea {

struct Individual {
    std::vector<double> objectives;
    std::any genotype;

    double crowding_distance = 0.0;
};

} // namespace UavMoea