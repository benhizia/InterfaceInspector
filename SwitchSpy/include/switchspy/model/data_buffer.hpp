#pragma once

#include "switchspy/common/types.hpp"
#include <vector>
#include <mutex>
#include <deque>
#include <functional>

namespace switchspy {
namespace model {

/**
 * Thread-safe ring buffer for packet data
 * Supports both real-time and deferred modes
 * Includes flow control monitoring and benchmarking
 */
class DataBuffer {
public:
    using OverflowCallback = std::function<void(size_t current_size, size_t max_size)>;
    using PressureCallback = std::function<void(size_t size, size_t capacity)>;

    explicit DataBuffer(size_t max_size = 10 * 1024 * 1024);

    void push(const PacketInfo& packet);
    bool pop(PacketInfo& packet);
    bool peek(PacketInfo& packet) const;

    size_t size() const;
    size_t getCurrentBytes() const { return current_size_; }
    size_t getMaxBytes() const { return max_size_; }
    bool empty() const;
    void clear();

    // Get buffer utilization (0.0 to 1.0)
    double getUtilization() const;

    // Check if buffer is under pressure
    bool isUnderPressure(double threshold = 0.75) const;

    // Get number of dropped packets due to overflow
    uint64_t getDroppedPackets() const { return dropped_packets_; }

    std::vector<PacketInfo> getAll() const;

    // Set callbacks for monitoring
    void setOverflowCallback(OverflowCallback callback) { overflow_callback_ = callback; }
    void setPressureCallback(PressureCallback callback) { pressure_callback_ = callback; }

private:
    mutable std::mutex mutex_;
    std::deque<PacketInfo> buffer_;
    size_t max_size_;
    size_t current_size_;
    uint64_t dropped_packets_;

    OverflowCallback overflow_callback_;
    PressureCallback pressure_callback_;
};

} // namespace model
} // namespace switchspy
