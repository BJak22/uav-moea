#include "variations/SbxVariation.h"

#include <algorithm>
#include <cmath>

namespace UavMoea::variations {

SbxVariation::SbxVariation(double pc, double pm, double eta_c, double eta_m, double lower_bound, double upper_bound)
    : m_pc(pc), m_pm(pm), m_eta_c(eta_c), m_eta_m(eta_m), m_lower_bound(lower_bound), m_upper_bound(upper_bound),
      m_rng(std::random_device{}()) {}

uint16_t SbxVariation::getRequiredParentsCount(uint16_t lambda) const {
    return lambda + (lambda % 2);
}

std::vector<Individual> SbxVariation::generateOffspring(const std::vector<Individual>& parents) {
    std::vector<Individual> offsprings;
    offsprings.reserve(parents.size());

    std::uniform_real_distribution<double> dist(0.0, 1.0);

    for (size_t i = 0; i + 1 < parents.size(); i += 2) {
        Individual child1 = parents[i];
        Individual child2 = parents[i + 1];

        auto x1 = std::any_cast<std::vector<double>>(child1.genotype);
        auto x2 = std::any_cast<std::vector<double>>(child2.genotype);

        if (dist(m_rng) <= m_pc) {
            applySbx(x1, x2);
        }

        applyPolynomialMutation(x1);
        applyPolynomialMutation(x2);

        child1.genotype = x1;
        child1.objectives.clear();

        child2.genotype = x2;
        child2.objectives.clear();

        offsprings.push_back(child1);
        offsprings.push_back(child2);
    }

    return offsprings;
}

void SbxVariation::applySbx(std::vector<double>& x1, std::vector<double>& x2) const {
    std::uniform_real_distribution<double> dist(0.0, 1.0);

    for (size_t i = 0; i < x1.size(); ++i) {
        if (dist(m_rng) <= 0.5) {
            double u = dist(m_rng);
            double beta_q = 0.0;

            if (u <= 0.5) {
                beta_q = std::pow(2.0 * u, 1.0 / (m_eta_c + 1.0));
            } else {
                beta_q = std::pow(1.0 / (2.0 * (1.0 - u)), 1.0 / (m_eta_c + 1.0));
            }

            double v1 = 0.5 * ((1.0 + beta_q) * x1[i] + (1.0 - beta_q) * x2[i]);
            double v2 = 0.5 * ((1.0 - beta_q) * x1[i] + (1.0 + beta_q) * x2[i]);

            x1[i] = std::clamp(v1, m_lower_bound, m_upper_bound);
            x2[i] = std::clamp(v2, m_lower_bound, m_upper_bound);
        }
    }
}

void SbxVariation::applyPolynomialMutation(std::vector<double>& x) const {
    std::uniform_real_distribution<double> dist(0.0, 1.0);

    for (size_t i = 0; i < x.size(); ++i) {
        if (dist(m_rng) <= m_pm) {
            double u = dist(m_rng);
            double delta_q = 0.0;

            if (u < 0.5) {
                delta_q = std::pow(2.0 * u, 1.0 / (m_eta_m + 1.0)) - 1.0;
            } else {
                delta_q = 1.0 - std::pow(2.0 * (1.0 - u), 1.0 / (m_eta_m + 1.0));
            }

            x[i] += delta_q * (m_upper_bound - m_lower_bound);
            x[i] = std::clamp(x[i], m_lower_bound, m_upper_bound);
        }
    }
}

} // namespace UavMoea::variations