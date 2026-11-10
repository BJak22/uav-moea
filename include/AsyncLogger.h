#pragma once
#include <atomic>
#include <condition_variable>
#include <fstream>
#include <mutex>
#include <queue>
#include <string>
#include <thread>
#include <vector>

namespace UavMoea {

struct Snapshot {
    int experiment_id;
    int evaluation;
    std::vector<std::vector<double>> objectives;
};

class AsyncLogger {
public:
    explicit AsyncLogger(const std::string& filename);
    ~AsyncLogger();

    void log(int experiment_id, int evaluation, const std::vector<std::vector<double>>& objectives);

private:
    void processQueue();

    std::queue<Snapshot> m_queue;
    std::mutex m_mutex;
    std::condition_variable m_cv;
    std::thread m_worker;
    std::atomic<bool> m_running;
    std::ofstream m_file;
};

} // namespace UavMoea