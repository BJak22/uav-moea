#include "core/AsyncLogger.h"
#include <iostream>

namespace UavMoea {

AsyncLogger::AsyncLogger(const std::string& filename) : m_running(true) {
    m_file.open(filename);
    if (m_file.is_open()) {
        m_file << "experiment_id,evaluation,f1,f2\n";
    }
    m_worker = std::thread(&AsyncLogger::processQueue, this);
}

AsyncLogger::~AsyncLogger() {
    m_running = false;
    m_cv.notify_one();
    if (m_worker.joinable()) {
        m_worker.join();
    }
    if (m_file.is_open()) {
        m_file.close();
    }
}

void AsyncLogger::log(int experiment_id, int evaluation, const std::vector<std::vector<double>>& objectives) {
    Snapshot snap{experiment_id, evaluation, objectives};

    {
        std::lock_guard<std::mutex> lock(m_mutex);
        m_queue.push(std::move(snap));
    }
    m_cv.notify_one();
}

void AsyncLogger::processQueue() {
    while (m_running || !m_queue.empty()) {
        std::unique_lock<std::mutex> lock(m_mutex);
        m_cv.wait(lock, [this] { return !m_queue.empty() || !m_running; });

        while (!m_queue.empty()) {
            Snapshot snap = std::move(m_queue.front());
            m_queue.pop();
            lock.unlock();

            if (m_file.is_open()) {
                for (const auto& obj : snap.objectives) {
                    m_file << snap.experiment_id << "," << snap.evaluation << ",";
                    for (size_t i = 0; i < obj.size(); ++i) {
                        m_file << obj[i] << (i == obj.size() - 1 ? "" : ",");
                    }
                    m_file << "\n";
                }
            }
            lock.lock();
        }
    }
}

} // namespace UavMoea