#pragma once

#include <random>

#include "IVariation.h"

namespace UavMoea::variations {

class SbxVariation : public IVariation {
public:
    SbxVariation(double pc = 0.9, double pm = 1.0 / 30.0, double eta_c = 20.0, double eta_m = 20.0,
                 double lower_bound = 0.0, double upper_bound = 1.0);

    uint16_t getRequiredParentsCount(uint16_t lambda) const override;
    std::vector<Individual> generateOffspring(const std::vector<Individual>& parents) override;

private:
    double m_pc;
    double m_pm;
    double m_eta_c;
    double m_eta_m;
    double m_lower_bound;
    double m_upper_bound;

    mutable std::mt19937 m_rng;

    void applySbx(std::vector<double>& x1, std::vector<double>& x2) const;
    void applyPolynomialMutation(std::vector<double>& x) const;
};

} // namespace UavMoea::variations