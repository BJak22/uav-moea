#pragma once

#include <any>
#include <vector>

namespace UavMoea {

struct Individual {
    std::vector<double> objectives;
    std::any genotype;

    double crowdingDistance = 0.0;
};

} // namespace UavMoea