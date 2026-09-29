#pragma once
#include "switchspy/model/connection_state.hpp"
#include <map>
#include <memory>

namespace switchspy { namespace controller {
class ConnectionManager {
public:
    static ConnectionManager& instance();
    void addConnection(const std::string& name);
    std::shared_ptr<model::ConnectionStateModel> getConnection(const std::string& name);
    const std::map<std::string, std::shared_ptr<model::ConnectionStateModel>>& getAllConnections() const { return connections_; }
private:
    ConnectionManager() = default;
    std::map<std::string, std::shared_ptr<model::ConnectionStateModel>> connections_;
};
}} // namespace
