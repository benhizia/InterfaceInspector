#include "switchspy/core/network_manager.hpp"
namespace switchspy { namespace core {
NetworkManager& NetworkManager::instance() { static NetworkManager inst; return inst; }
void NetworkManager::addConnection(std::shared_ptr<NetworkConnection> conn) { connections_.push_back(conn); }
void NetworkManager::removeConnection(const std::string& name) {}
void NetworkManager::pollAll() {}
void NetworkManager::initialize() { platform::initialize_networking(); }
void NetworkManager::shutdown() { platform::cleanup_networking(); }
}} // namespace
