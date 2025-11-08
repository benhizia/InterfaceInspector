#include "switchspy/core/tcp_connection.hpp"
#include <cstring>

namespace switchspy {
namespace core {

TCPConnection::TCPConnection()
    : NetworkConnection(ConnectionType::TCP)
{
}

TCPConnection::TCPConnection(socket_t accepted_socket, const NetworkAddress& remote)
    : NetworkConnection(ConnectionType::TCP)
{
    socket_ = accepted_socket;
    remote_address_ = remote;
    setState(ConnectionState::Connected);

    // Set non-blocking
    platform::set_non_blocking(socket_);
}

bool TCPConnection::connect(const NetworkAddress& remote) {
    std::lock_guard<std::mutex> lock(mutex_);

    if (!createSocket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) {
        return false;
    }

    remote_address_ = remote;

    // Setup address structure
    struct sockaddr_in addr;
    std::memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port = htons(remote.port);

    if (inet_pton(AF_INET, remote.host.c_str(), &addr.sin_addr) <= 0) {
        reportError("Invalid address: " + remote.host);
        close();
        return false;
    }

    setState(ConnectionState::Connecting);

    // Non-blocking connect
    int result = ::connect(socket_, reinterpret_cast<struct sockaddr*>(&addr), sizeof(addr));

    if (result < 0) {
        int error = platform::get_last_error();

        // Check if connection is in progress (expected for non-blocking)
#ifdef SWITCHSPY_PLATFORM_WINDOWS
        if (error == WSAEWOULDBLOCK || error == WSAEINPROGRESS)
#else
        if (error == EINPROGRESS)
#endif
        {
            // Connection in progress, will complete asynchronously
            return true;
        } else {
            reportError("Connect failed: " + std::to_string(error));
            close();
            return false;
        }
    }

    setState(ConnectionState::Connected);
    return true;
}

bool TCPConnection::listen(const NetworkAddress& local) {
    std::lock_guard<std::mutex> lock(mutex_);

    if (!createSocket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) {
        return false;
    }

    local_address_ = local;

    // Setup address structure
    struct sockaddr_in addr;
    std::memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port = htons(local.port);

    if (local.host.empty() || local.host == "0.0.0.0") {
        addr.sin_addr.s_addr = INADDR_ANY;
    } else {
        if (inet_pton(AF_INET, local.host.c_str(), &addr.sin_addr) <= 0) {
            reportError("Invalid address: " + local.host);
            close();
            return false;
        }
    }

    // Bind socket
    if (bind(socket_, reinterpret_cast<struct sockaddr*>(&addr), sizeof(addr)) < 0) {
        reportError("Bind failed: " + std::to_string(platform::get_last_error()));
        close();
        return false;
    }

    // Start listening
    if (::listen(socket_, SOMAXCONN) < 0) {
        reportError("Listen failed: " + std::to_string(platform::get_last_error()));
        close();
        return false;
    }

    setState(ConnectionState::Listening);
    return true;
}

std::unique_ptr<TCPConnection> TCPConnection::accept() {
    struct sockaddr_in client_addr;
    socklen_t client_len = sizeof(client_addr);

    socket_t client_socket = ::accept(socket_,
                                      reinterpret_cast<struct sockaddr*>(&client_addr),
                                      &client_len);

    if (client_socket == INVALID_SOCKET_FD) {
        int error = platform::get_last_error();
        if (!platform::is_would_block(error)) {
            reportError("Accept failed: " + std::to_string(error));
        }
        return nullptr;
    }

    // Get client address
    char client_ip[INET_ADDRSTRLEN];
    inet_ntop(AF_INET, &client_addr.sin_addr, client_ip, INET_ADDRSTRLEN);

    NetworkAddress client_address(client_ip, ntohs(client_addr.sin_port));

    return std::make_unique<TCPConnection>(client_socket, client_address);
}

ssize_t TCPConnection::send(const byte* data, size_t size) {
    if (state_ != ConnectionState::Connected) {
        return -1;
    }

    ssize_t sent = ::send(socket_, reinterpret_cast<const char*>(data), size, 0);

    if (sent < 0) {
        int error = platform::get_last_error();
        if (!platform::is_would_block(error)) {
            reportError("Send failed: " + std::to_string(error));
        }
    }

    return sent;
}

ssize_t TCPConnection::receive(byte* buffer, size_t size) {
    if (state_ != ConnectionState::Connected) {
        return -1;
    }

    ssize_t received = ::recv(socket_, reinterpret_cast<char*>(buffer), size, 0);

    if (received < 0) {
        int error = platform::get_last_error();
        if (!platform::is_would_block(error)) {
            reportError("Receive failed: " + std::to_string(error));
        }
    } else if (received == 0) {
        // Connection closed by peer
        setState(ConnectionState::Disconnected);
    }

    return received;
}

} // namespace core
} // namespace switchspy
