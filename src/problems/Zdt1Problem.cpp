#include "problems/Zdt1Problem.h"

#include <cmath>
#include <numeric>
#include <random>

namespace UavMoea::problems {

Zdt1Problem::Zdt1Problem(int dimensions) : m_dimensions(dimensions), m_rng(std::random_device{}()) {}

Individual Zdt1Problem::generateRandom() const {
    std::uniform_real_distribution<double> dist(0.0, 1.0);

    std::vector<double> variables(m_dimensions);
    for (int i = 0; i < m_dimensions; ++i) {
        variables[i] = dist(m_rng);
    }

    Individual ind;
    ind.genotype = variables;

    return ind;
}

void Zdt1Problem::evaluate(Individual& individual) const {
    const auto& x = std::any_cast<std::vector<double>>(individual.genotype);
    double f1 = x[0];
    double sum = 0.0;

    for (int i = 1; i < m_dimensions; ++i) {
        sum += x[i];
    }

    double g = 1.0 + 9.0 * (sum / (m_dimensions - 1));
    double h = 1.0 - std::sqrt(f1 / g);
    double f2 = g * h;

    individual.objectives = {f1, f2};
}

} // namespace UavMoea::problems