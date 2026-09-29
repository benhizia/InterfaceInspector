#pragma once

#include "switchspy/common/types.hpp"
#include "switchspy/model/data_buffer.hpp"
#include <memory>

namespace switchspy {
namespace model {

class ConnectionStateModel {
public:
    explicit ConnectionStateModel(const std::string& name);
    
    std::string getName() const { return name_; }
    ConnectionState getState() const { return state_; }
    void setState(ConnectionState state) { state_ = state; }
    
    ConnectionStats& getStats() { return stats_; }
    const ConnectionStats& getStats() const { return stats_; }
    
    DataBuffer& getBuffer() { return buffer_; }
    const DataBuffer& getBuffer() const { return buffer_; }

private:
    std::string name_;
    ConnectionState state_;
    ConnectionStats stats_;
    DataBuffer buffer_;
};

} // namespace model
} // namespace switchspy
