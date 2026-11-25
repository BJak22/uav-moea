#include "archivers/MultiLevelGridArchiver.h"

#include <algorithm>
#include <cmath>
#include <random>

namespace UavMoea::archivers {

MultiLevelGridArchiver::MultiLevelGridArchiver(uint16_t capacity)
    : m_capacity(capacity), m_rng(std::random_device{}()) {
    m_archive.reserve(capacity + 1);
}

const std::vector<Individual>& MultiLevelGridArchiver::getArchive() const {
    return m_archive;
}

std::vector<int64_t> MultiLevelGridArchiver::getBox(const std::vector<double>& obj, int b) const {
    std::vector<int64_t> box(obj.size());
    for (size_t i = 0; i < obj.size(); ++i) {
        box[i] = static_cast<int64_t>(std::floor(std::ldexp(obj[i], -b)));
    }
    return box;
}

bool MultiLevelGridArchiver::boxDominatesOrEqual(const std::vector<int64_t>& boxA,
                                                 const std::vector<int64_t>& boxB) const {
    for (size_t i = 0; i < boxA.size(); ++i) {
        if (boxA[i] > boxB[i])
            return false;
    }
    return true;
}

void MultiLevelGridArchiver::update(const std::vector<Individual>& offspring) {

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
        if (rejected)
            continue;

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

        double max_val = 0.0;
        for (const auto& ind : next_archive) {
            for (double val : ind.objectives) {
                if (std::abs(val) > max_val)
                    max_val = std::abs(val);
            }
        }

        int bar_b = 0;
        if (max_val > 0.0) {
            bar_b = static_cast<int>(std::floor(std::log2(max_val))) + 1;
        }

        std::vector<size_t> final_D;

        int low = bar_b - 52;
        int high = bar_b;

        while (low <= high) {
            int mid = low + (high - low) / 2;
            std::vector<size_t> current_D;

            std::vector<std::vector<int64_t>> boxes(next_archive.size());
            for (size_t i = 0; i < next_archive.size(); ++i) {
                boxes[i] = getBox(next_archive[i].objectives, mid);
            }

            for (size_t i = 0; i < next_archive.size(); ++i) {
                for (size_t j = 0; j < next_archive.size(); ++j) {
                    if (i == j)
                        continue;
                    if (boxDominatesOrEqual(boxes[i], boxes[j])) {
                        current_D.push_back(j);
                    }
                }
            }

            if (!current_D.empty()) {
                final_D = std::move(current_D);
                high = mid - 1;
            } else {
                low = mid + 1;
            }
        }

        size_t s_idx = next_archive.size() - 1;
        size_t to_remove = s_idx;

        if (!final_D.empty()) {
            std::sort(final_D.begin(), final_D.end());
            final_D.erase(std::unique(final_D.begin(), final_D.end()), final_D.end());

            auto it = std::find(final_D.begin(), final_D.end(), s_idx);
            if (it != final_D.end()) {
                to_remove = s_idx;
            } else {
                std::uniform_int_distribution<size_t> dist(0, final_D.size() - 1);
                to_remove = final_D[dist(m_rng)];
            }
        }

        next_archive.erase(next_archive.begin() + to_remove);
        m_archive = std::move(next_archive);
    }
}

} // namespace UavMoea::archivers