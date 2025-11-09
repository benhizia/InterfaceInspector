#pragma once

#include <chrono>
#include <string>
#include <map>
#include <vector>
#include <mutex>

namespace switchspy {
namespace benchmark {

using clock_type = std::chrono::high_resolution_clock;
using time_point = std::chrono::high_resolution_clock::time_point;
using duration_us = std::chrono::microseconds;

// Performance metrics for a specific operation
struct PerformanceMetrics {
    std::string name;
    uint64_t call_count = 0;
    uint64_t total_time_us = 0;
    uint64_t min_time_us = UINT64_MAX;
    uint64_t max_time_us = 0;
    uint64_t last_time_us = 0;

    double getAverageUs() const {
        return call_count > 0 ? static_cast<double>(total_time_us) / call_count : 0.0;
    }

    void update(uint64_t time_us) {
        call_count++;
        total_time_us += time_us;
        last_time_us = time_us;
        if (time_us < min_time_us) min_time_us = time_us;
        if (time_us > max_time_us) max_time_us = time_us;
    }

    void reset() {
        call_count = 0;
        total_time_us = 0;
        min_time_us = UINT64_MAX;
        max_time_us = 0;
        last_time_us = 0;
    }
};

// RAII timer for automatic benchmarking
class ScopedTimer {
public:
    explicit ScopedTimer(const std::string& name);
    ~ScopedTimer();

    // Get elapsed time without stopping
    uint64_t elapsed_us() const;

private:
    std::string name_;
    time_point start_;
};

// Performance monitor singleton
class PerformanceMonitor {
public:
    static PerformanceMonitor& instance();

    // Record a measurement
    void record(const std::string& name, uint64_t time_us);

    // Get metrics for a specific operation
    PerformanceMetrics getMetrics(const std::string& name) const;

    // Get all metrics
    std::map<std::string, PerformanceMetrics> getAllMetrics() const;

    // Reset a specific metric
    void reset(const std::string& name);

    // Reset all metrics
    void resetAll();

    // Check if operation is slow (exceeds threshold)
    bool isSlow(const std::string& name, uint64_t threshold_us) const;

    // Get operations exceeding threshold
    std::vector<std::string> getSlowOperations(uint64_t threshold_us) const;

private:
    PerformanceMonitor() = default;
    ~PerformanceMonitor() = default;

    PerformanceMonitor(const PerformanceMonitor&) = delete;
    PerformanceMonitor& operator=(const PerformanceMonitor&) = delete;

    mutable std::mutex mutex_;
    std::map<std::string, PerformanceMetrics> metrics_;
};

// Benchmark categories for hot paths
namespace categories {
    constexpr const char* NETWORK_SEND = "network.send";
    constexpr const char* NETWORK_RECV = "network.recv";
    constexpr const char* RELAY_FORWARD = "relay.forward";
    constexpr const char* BUFFER_PUSH = "buffer.push";
    constexpr const char* BUFFER_POP = "buffer.pop";
    constexpr const char* SERIALIZATION_WRITE = "serialization.write";
    constexpr const char* SERIALIZATION_READ = "serialization.read";
    constexpr const char* VISUALIZATION_UPDATE = "visualization.update";
    constexpr const char* GUI_RENDER = "gui.render";
    constexpr const char* PACKET_PROCESSING = "packet.process";
}

// Performance thresholds (in microseconds)
namespace thresholds {
    constexpr uint64_t NETWORK_IO = 1000;        // 1ms
    constexpr uint64_t RELAY_LATENCY = 500;      // 500us
    constexpr uint64_t BUFFER_OPERATION = 100;   // 100us
    constexpr uint64_t SERIALIZATION = 500;      // 500us
    constexpr uint64_t VISUALIZATION = 5000;     // 5ms
    constexpr uint64_t GUI_FRAME = 16666;        // 16.666ms (60 FPS)
}

// Helper macro for scoped timing
#define BENCHMARK_SCOPE(name) switchspy::benchmark::ScopedTimer _benchmark_timer_##__LINE__(name)
#define BENCHMARK_SCOPE_CAT(category) BENCHMARK_SCOPE(category)

} // namespace benchmark
} // namespace switchspy
