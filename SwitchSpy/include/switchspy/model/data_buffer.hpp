#pragma once

#include "switchspy/common/types.hpp"
#include <vector>
#include <mutex>
#include <deque>

namespace switchspy {
namespace model {

/**
 * Thread-safe ring buffer for packet data
 * Supports both real-time and deferred modes
 */
class DataBuffer {
public:
    explicit DataBuffer(size_t max_size = 10 * 1024 * 1024);

    void push(const PacketInfo& packet);
    bool pop(PacketInfo& packet);
    bool peek(PacketInfo& packet) const;
    
    size_t size() const;
    bool empty() const;
    void clear();

    std::vector<PacketInfo> getAll() const;

private:
    mutable std::mutex mutex_;
    std::deque<PacketInfo> buffer_;
    size_t max_size_;
    size_t current_size_;
};

} // namespace model
} // namespace switchspy
