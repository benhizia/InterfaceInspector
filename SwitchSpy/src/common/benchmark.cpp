#include "switchspy/common/benchmark.hpp"
#include "switchspy/common/logger.hpp"

namespace switchspy {
namespace benchmark {

ScopedTimer::ScopedTimer(const std::string& name)
    : name_(name)
    , start_(clock_type::now())
{
}

ScopedTimer::~ScopedTimer() {
    auto end = clock_type::now();
    auto duration = std::chrono::duration_cast<duration_us>(end - start_);
    PerformanceMonitor::instance().record(name_, duration.count());
}

uint64_t ScopedTimer::elapsed_us() const {
    auto now = clock_type::now();
    auto duration = std::chrono::duration_cast<duration_us>(now - start_);
    return duration.count();
}

PerformanceMonitor& PerformanceMonitor::instance() {
    static PerformanceMonitor instance;
    return instance;
}

void PerformanceMonitor::record(const std::string& name, uint64_t time_us) {
    std::lock_guard<std::mutex> lock(mutex_);

    auto& metrics = metrics_[name];
    if (metrics.name.empty()) {
        metrics.name = name;
    }

    metrics.update(time_us);

    // Check thresholds and log warnings
    uint64_t threshold = 0;
    if (name.find("network") == 0) {
        threshold = thresholds::NETWORK_IO;
    } else if (name.find("relay") == 0) {
        threshold = thresholds::RELAY_LATENCY;
    } else if (name.find("buffer") == 0) {
        threshold = thresholds::BUFFER_OPERATION;
    } else if (name.find("serialization") == 0) {
        threshold = thresholds::SERIALIZATION;
    } else if (name.find("visualization") == 0) {
        threshold = thresholds::VISUALIZATION;
    } else if (name.find("gui") == 0) {
        threshold = thresholds::GUI_FRAME;
    }

    if (threshold > 0 && time_us > threshold) {
        char msg[256];
        snprintf(msg, sizeof(msg), "Slow operation '%s': %llu us (threshold: %llu us)",
                 name.c_str(), (unsigned long long)time_us, (unsigned long long)threshold);
        logging::Logger::instance().warning("Performance", msg);
    }
}

PerformanceMetrics PerformanceMonitor::getMetrics(const std::string& name) const {
    std::lock_guard<std::mutex> lock(mutex_);

    auto it = metrics_.find(name);
    if (it != metrics_.end()) {
        return it->second;
    }

    PerformanceMetrics empty;
    empty.name = name;
    return empty;
}

std::map<std::string, PerformanceMetrics> PerformanceMonitor::getAllMetrics() const {
    std::lock_guard<std::mutex> lock(mutex_);
    return metrics_;
}

void PerformanceMonitor::reset(const std::string& name) {
    std::lock_guard<std::mutex> lock(mutex_);

    auto it = metrics_.find(name);
    if (it != metrics_.end()) {
        it->second.reset();
    }
}

void PerformanceMonitor::resetAll() {
    std::lock_guard<std::mutex> lock(mutex_);
    metrics_.clear();
}

bool PerformanceMonitor::isSlow(const std::string& name, uint64_t threshold_us) const {
    std::lock_guard<std::mutex> lock(mutex_);

    auto it = metrics_.find(name);
    if (it != metrics_.end()) {
        return it->second.last_time_us > threshold_us;
    }

    return false;
}

std::vector<std::string> PerformanceMonitor::getSlowOperations(uint64_t threshold_us) const {
    std::lock_guard<std::mutex> lock(mutex_);

    std::vector<std::string> slow_ops;
    for (const auto& [name, metrics] : metrics_) {
        if (metrics.getAverageUs() > threshold_us) {
            slow_ops.push_back(name);
        }
    }

    return slow_ops;
}

} // namespace benchmark
} // namespace switchspy
