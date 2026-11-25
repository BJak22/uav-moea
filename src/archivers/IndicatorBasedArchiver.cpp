#include "archivers/IndicatorBasedArchiver.h"

#include <algorithm>

namespace UavMoea::archivers {

IndicatorBasedArchiver::IndicatorBasedArchiver(uint16_t capacity, std::unique_ptr<indicators::IIndicator> indicator)
    : m_capacity(capacity), m_indicator(std::move(indicator)) {
    m_archive.reserve(m_capacity + 1);
}

const std::vector<Individual>& IndicatorBasedArchiver::getArchive() const {
    return m_archive;
}

void IndicatorBasedArchiver::update(const std::vector<Individual>& offspring) {
    for (const auto& s : offspring) {
        bool rejected = false;
        for (const auto& a : m_archive) {
            bool weakly_dominates = true;
            for (size_t i = 0; i < a.objectives.size(); ++i) {
                if (a.objectives[i] > s.objectives[i]) {
                    weakly_dominates = false;
                    break;
                }
            }
            if (weakly_dominates) {
                rejected = true;
                break;
            }
        }
        if (rejected) {
            continue;
        }

        std::vector<Individual> next_archive;
        next_archive.reserve(m_archive.size() + 1);
        for (auto& a : m_archive) {
            if (!s.dominates(a)) {
                next_archive.push_back(std::move(a));
            }
        }
        next_archive.push_back(s);

        if (next_archive.size() <= m_capacity) {
            m_archive = std::move(next_archive);
            continue;
        }

        m_indicator->updateState(next_archive);
        std::vector<double> contributions = m_indicator->calculateContributions(next_archive);

        const size_t s_idx = next_archive.size() - 1;
        auto min_old_it = std::min_element(contributions.begin(), contributions.begin() + s_idx);

        if (contributions[s_idx] <= *min_old_it) {
            next_archive.pop_back();
            m_archive = std::move(next_archive);
        } else {
            size_t remove_idx = static_cast<size_t>(std::distance(contributions.begin(), min_old_it));
            next_archive.erase(next_archive.begin() + remove_idx);
            m_archive = std::move(next_archive);
        }
    }
}

} // namespace UavMoea::archivers