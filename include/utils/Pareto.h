#pragma once

#include <cstddef>
#include <vector>

#include "core/Individual.h"

namespace UavMoea::utils {

std::vector<std::vector<size_t>> fastNonDominatedSort(const std::vector<Individual>& population);

} // namespace UavMoea::utils