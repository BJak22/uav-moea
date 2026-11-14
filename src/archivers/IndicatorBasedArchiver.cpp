#include "archivers/IndicatorBasedArchiver.h"
#include "utils/Pareto.h"
#include <algorithm>

namespace UavMoea::archivers {

IndicatorBasedArchiver::IndicatorBasedArchiver(uint16_t capacity, std::unique_ptr<indicators::IIndicator> indicator)
    : m_capacity(capacity), m_indicator(std::move(indicator)) {
    m_archive.reserve(m_capacity);
}

const std::vector<Individual>& IndicatorBasedArchiver::getArchive() const {
    return m_archive;
}

void IndicatorBasedArchiver::update(const std::vector<Individual>& offspring) {
    if (m_archive.empty()) {
        m_archive = offspring;
        return;
    }

    std::vector<Individual> combined = m_archive;
    combined.insert(combined.end(), offspring.begin(), offspring.end());

    if (combined.size() <= m_capacity) {
        m_archive = std::move(combined);
        return;
    }

    std::vector<std::vector<size_t>> fronts = utils::fastNonDominatedSort(combined);

    std::vector<Individual> next_archive;
    next_archive.reserve(m_capacity);

    size_t current_front_idx = 0;

    while (current_front_idx < fronts.size() && next_archive.size() + fronts[current_front_idx].size() <= m_capacity) {

        for (size_t idx : fronts[current_front_idx]) {
            next_archive.push_back(std::move(combined[idx]));
        }
        current_front_idx++;
    }

    if (next_archive.size() < m_capacity && current_front_idx < fronts.size()) {

        std::vector<Individual> last_front;
        for (size_t idx : fronts[current_front_idx]) {
            last_front.push_back(std::move(combined[idx]));
        }

        size_t needed = m_capacity - next_archive.size();

        while (last_front.size() > needed) {
            std::vector<Individual> eval_set = next_archive;
            eval_set.insert(eval_set.end(), last_front.begin(), last_front.end());

            m_indicator->updateState(eval_set);
            std::vector<double> contributions = m_indicator->calculateContributions(eval_set);

            size_t offset = next_archive.size();
            auto min_it = std::min_element(contributions.begin() + offset, contributions.end());
            size_t min_idx_in_last_front = std::distance(contributions.begin() + offset, min_it);
            last_front.erase(last_front.begin() + min_idx_in_last_front);
        }

        for (auto& ind : last_front) {
            next_archive.push_back(std::move(ind));
        }
    }

    m_archive = std::move(next_archive);
}

} // namespace UavMoea::archivers