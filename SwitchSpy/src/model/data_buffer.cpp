#include "switchspy/model/data_buffer.hpp"
#include "switchspy/common/benchmark.hpp"
#include "switchspy/common/logger.hpp"

namespace switchspy {
namespace model {

DataBuffer::DataBuffer(size_t max_size)
    : max_size_(max_size)
    , current_size_(0)
    , dropped_packets_(0)
{}

void DataBuffer::push(const PacketInfo& packet) {
    BENCHMARK_SCOPE_CAT(benchmark::categories::BUFFER_PUSH);

    std::lock_guard<std::mutex> lock(mutex_);

    // Check if we need to drop old packets
    bool overflow = false;
    while (current_size_ + packet.size > max_size_ && !buffer_.empty()) {
        current_size_ -= buffer_.front().size;
        buffer_.pop_front();
        dropped_packets_++;
        overflow = true;
    }

    // If still can't fit after dropping all packets, the packet is too large
    if (current_size_ + packet.size > max_size_) {
        dropped_packets_++;
        if (overflow_callback_) {
            overflow_callback_(current_size_, max_size_);
        }
        LOG_WARNING("DataBuffer", "Packet too large for buffer, dropping");
        return;
    }

    buffer_.push_back(packet);
    current_size_ += packet.size;

    // Notify about overflow if it occurred
    if (overflow && overflow_callback_) {
        overflow_callback_(current_size_, max_size_);
    }

    // Check pressure and notify
    if (pressure_callback_) {
        pressure_callback_(current_size_, max_size_);
    }

    // Log warning if buffer is getting full
    double utilization = getUtilization();
    if (utilization > 0.9) {
        static auto last_warning = std::chrono::steady_clock::now();
        auto now = std::chrono::steady_clock::now();
        if (std::chrono::duration_cast<std::chrono::seconds>(now - last_warning).count() > 5) {
            char msg[128];
            snprintf(msg, sizeof(msg), "Buffer at %.1f%% capacity (%zu/%zu bytes)",
                     utilization * 100, current_size_, max_size_);
            LOG_WARNING("DataBuffer", msg);
            last_warning = now;
        }
    }
}

bool DataBuffer::pop(PacketInfo& packet) {
    BENCHMARK_SCOPE_CAT(benchmark::categories::BUFFER_POP);

    std::lock_guard<std::mutex> lock(mutex_);
    if (buffer_.empty()) return false;

    packet = buffer_.front();
    buffer_.pop_front();
    current_size_ -= packet.size;
    return true;
}

bool DataBuffer::peek(PacketInfo& packet) const {
    std::lock_guard<std::mutex> lock(mutex_);
    if (buffer_.empty()) return false;
    packet = buffer_.front();
    return true;
}

size_t DataBuffer::size() const {
    std::lock_guard<std::mutex> lock(mutex_);
    return buffer_.size();
}

bool DataBuffer::empty() const {
    std::lock_guard<std::mutex> lock(mutex_);
    return buffer_.empty();
}

void DataBuffer::clear() {
    std::lock_guard<std::mutex> lock(mutex_);
    buffer_.clear();
    current_size_ = 0;
}

double DataBuffer::getUtilization() const {
    if (max_size_ == 0) return 0.0;
    return static_cast<double>(current_size_) / max_size_;
}

bool DataBuffer::isUnderPressure(double threshold) const {
    return getUtilization() >= threshold;
}

std::vector<PacketInfo> DataBuffer::getAll() const {
    std::lock_guard<std::mutex> lock(mutex_);
    return std::vector<PacketInfo>(buffer_.begin(), buffer_.end());
}

} // namespace model
} // namespace switchspy
