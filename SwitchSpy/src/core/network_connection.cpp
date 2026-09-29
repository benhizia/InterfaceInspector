#include "switchspy/core/network_connection.hpp"
#include <iostream>

namespace switchspy {
namespace core {

NetworkConnection::NetworkConnection(ConnectionType type)
    : socket_(INVALID_SOCKET_FD)
    , type_(type)
    , state_(ConnectionState::Disconnected)
{
}

NetworkConnection::~NetworkConnection() {
    close();
}

void NetworkConnection::close() {
    std::lock_guard<std::mutex> lock(mutex_);

    if (socket_ != INVALID_SOCKET_FD) {
        close_socket(socket_);
        socket_ = INVALID_SOCKET_FD;
    }

    setState(ConnectionState::Disconnected);
}

void NetworkConnection::setState(ConnectionState new_state) {
    ConnectionState old_state = state_.exchange(new_state);

    if (old_state != new_state && state_change_callback_) {
        state_change_callback_(old_state, new_state);
    }
}

void NetworkConnection::reportError(const std::string& error) {
    if (error_callback_) {
        error_callback_(error);
    }
    setState(ConnectionState::Error);
}

bool NetworkConnection::createSocket(int domain, int type, int protocol) {
    socket_ = socket(domain, type, protocol);

    if (socket_ == INVALID_SOCKET_FD) {
        reportError("Failed to create socket: " + std::to_string(platform::get_last_error()));
        return false;
    }

    // Set socket to non-blocking
    if (!platform::set_non_blocking(socket_)) {
        reportError("Failed to set non-blocking mode");
        close();
        return false;
    }

    // Enable address reuse
    int reuse = 1;
    if (setsockopt(socket_, SOL_SOCKET, SO_REUSEADDR,
                   reinterpret_cast<const char*>(&reuse), sizeof(reuse)) < 0) {
        reportError("Failed to set SO_REUSEADDR");
        // Non-fatal, continue
    }

    return true;
}

} // namespace core
} // namespace switchspy
