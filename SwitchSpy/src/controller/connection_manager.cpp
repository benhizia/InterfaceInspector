#include "switchspy/controller/connection_manager.hpp"
namespace switchspy { namespace controller {
ConnectionManager& ConnectionManager::instance() { static ConnectionManager inst; return inst; }
void ConnectionManager::addConnection(const std::string& name) {
    connections_[name] = std::make_shared<model::ConnectionStateModel>(name);
}
std::shared_ptr<model::ConnectionStateModel> ConnectionManager::getConnection(const std::string& name) {
    auto it = connections_.find(name);
    return it != connections_.end() ? it->second : nullptr;
}
}} // namespace
