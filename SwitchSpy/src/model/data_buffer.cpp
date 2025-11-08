#include "switchspy/model/data_buffer.hpp"

namespace switchspy {
namespace model {

DataBuffer::DataBuffer(size_t max_size)
    : max_size_(max_size), current_size_(0) {}

void DataBuffer::push(const PacketInfo& packet) {
    std::lock_guard<std::mutex> lock(mutex_);
    
    while (current_size_ + packet.size > max_size_ && !buffer_.empty()) {
        current_size_ -= buffer_.front().size;
        buffer_.pop_front();
    }
    
    buffer_.push_back(packet);
    current_size_ += packet.size;
}

bool DataBuffer::pop(PacketInfo& packet) {
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

std::vector<PacketInfo> DataBuffer::getAll() const {
    std::lock_guard<std::mutex> lock(mutex_);
    return std::vector<PacketInfo>(buffer_.begin(), buffer_.end());
}

} // namespace model
} // namespace switchspy
