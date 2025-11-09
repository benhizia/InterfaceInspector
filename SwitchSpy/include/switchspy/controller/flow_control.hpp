#pragma once

#include "switchspy/common/types.hpp"
#include <atomic>
#include <chrono>
#include <string>

namespace switchspy {
namespace controller {

/**
 * Flow control monitor
 * Detects rate mismatches, buffer pressure, and connection issues
 */
class FlowControlMonitor {
public:
    explicit FlowControlMonitor(const std::string& connection_name);

    // Update metrics
    void onPacketReceived(size_t bytes);
    void onPacketSent(size_t bytes);
    void onBufferPush(size_t buffer_size, size_t buffer_capacity);
    void onBufferPop();
    void onSendBlocked();
    void onReceiveBlocked();

    // Get current rates (bytes per second)
    double getReceiveRate() const { return receive_rate_bps_; }
    double getSendRate() const { return send_rate_bps_; }
    double getRateMismatch() const;  // Returns ratio (send/receive or receive/send)

    // Buffer pressure (0.0 to 1.0)
    double getBufferPressure() const { return buffer_pressure_; }

    // Check for issues
    bool isBufferOverflowing() const;
    bool isRateMismatched() const;
    bool isSenderFasterThanReceiver() const;
    bool isReceiverNotConnected() const;
    bool isSenderNotConnected() const;

    // Get diagnostic info
    struct DiagnosticInfo {
        std::string connection_name;
        double receive_rate_bps;
        double send_rate_bps;
        double rate_mismatch;
        double buffer_pressure;
        uint64_t blocked_send_count;
        uint64_t blocked_receive_count;
        uint64_t total_received_bytes;
        uint64_t total_sent_bytes;
        bool buffer_overflow_warning;
        bool rate_mismatch_warning;
        bool receiver_disconnected_warning;
        bool sender_disconnected_warning;
    };

    DiagnosticInfo getDiagnostics() const;

    // Reset counters
    void reset();

private:
    void updateRates();
    void checkAndLogWarnings();

    std::string connection_name_;

    // Rate tracking
    std::atomic<uint64_t> total_received_bytes_{0};
    std::atomic<uint64_t> total_sent_bytes_{0};
    std::atomic<uint64_t> bytes_received_last_second_{0};
    std::atomic<uint64_t> bytes_sent_last_second_{0};

    std::chrono::steady_clock::time_point last_rate_update_;
    double receive_rate_bps_{0.0};
    double send_rate_bps_{0.0};

    // Buffer tracking
    std::atomic<double> buffer_pressure_{0.0};
    std::atomic<uint64_t> buffer_push_count_{0};
    std::atomic<uint64_t> buffer_pop_count_{0};

    // Blocking tracking
    std::atomic<uint64_t> blocked_send_count_{0};
    std::atomic<uint64_t> blocked_receive_count_{0};

    std::chrono::steady_clock::time_point last_send_;
    std::chrono::steady_clock::time_point last_receive_;

    // Warning thresholds
    static constexpr double RATE_MISMATCH_THRESHOLD = 2.0;  // 2x rate difference
    static constexpr double BUFFER_PRESSURE_WARNING = 0.75;  // 75% full
    static constexpr double BUFFER_PRESSURE_CRITICAL = 0.90; // 90% full
    static constexpr int CONNECTION_TIMEOUT_MS = 5000;       // 5 seconds
};

} // namespace controller
} // namespace switchspy
