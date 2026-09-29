#pragma once
#include "switchspy/core/network_connection.hpp"
#include <vector>
#include <memory>

namespace switchspy { namespace core {
class NetworkManager {
public:
    static NetworkManager& instance();
    void addConnection(std::shared_ptr<NetworkConnection> conn);
    void removeConnection(const std::string& name);
    void pollAll();
    void initialize();
    void shutdown();
private:
    NetworkManager() = default;
    std::vector<std::shared_ptr<NetworkConnection>> connections_;
};
}} // namespace
