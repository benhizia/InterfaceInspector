#include "switchspy/core/udp_connection.hpp"
#include <cstring>

namespace switchspy {
namespace core {

UDPConnection::UDPConnection()
    : NetworkConnection(ConnectionType::UDP)
    , bound_(false)
{
}

bool UDPConnection::connect(const NetworkAddress& remote) {
    std::lock_guard<std::mutex> lock(mutex_);

    if (socket_ == INVALID_SOCKET_FD) {
        if (!createSocket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) {
            return false;
        }
    }

    remote_address_ = remote;

    // For UDP, "connect" just sets the default destination
    struct sockaddr_in addr;
    std::memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port = htons(remote.port);

    if (inet_pton(AF_INET, remote.host.c_str(), &addr.sin_addr) <= 0) {
        reportError("Invalid address: " + remote.host);
        return false;
    }

    if (::connect(socket_, reinterpret_cast<struct sockaddr*>(&addr), sizeof(addr)) < 0) {
        reportError("UDP connect failed: " + std::to_string(platform::get_last_error()));
        return false;
    }

    setState(ConnectionState::Connected);
    return true;
}

bool UDPConnection::listen(const NetworkAddress& local) {
    std::lock_guard<std::mutex> lock(mutex_);

    if (socket_ == INVALID_SOCKET_FD) {
        if (!createSocket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) {
            return false;
        }
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
            return false;
        }
    }

    // Bind socket
    if (bind(socket_, reinterpret_cast<struct sockaddr*>(&addr), sizeof(addr)) < 0) {
        reportError("Bind failed: " + std::to_string(platform::get_last_error()));
        return false;
    }

    bound_ = true;
    setState(ConnectionState::Listening);
    return true;
}

ssize_t UDPConnection::send(const byte* data, size_t size) {
    if (socket_ == INVALID_SOCKET_FD) {
        return -1;
    }

    // Send to "connected" destination
    ssize_t sent = ::send(socket_, reinterpret_cast<const char*>(data), size, 0);

    if (sent < 0) {
        int error = platform::get_last_error();
        if (!platform::is_would_block(error)) {
            reportError("Send failed: " + std::to_string(error));
        }
    }

    return sent;
}

ssize_t UDPConnection::sendTo(const byte* data, size_t size, const NetworkAddress& dest) {
    if (socket_ == INVALID_SOCKET_FD) {
        if (!createSocket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) {
            return -1;
        }
    }

    struct sockaddr_in addr;
    std::memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port = htons(dest.port);

    if (inet_pton(AF_INET, dest.host.c_str(), &addr.sin_addr) <= 0) {
        reportError("Invalid address: " + dest.host);
        return -1;
    }

    ssize_t sent = sendto(socket_, reinterpret_cast<const char*>(data), size, 0,
                          reinterpret_cast<struct sockaddr*>(&addr), sizeof(addr));

    if (sent < 0) {
        int error = platform::get_last_error();
        if (!platform::is_would_block(error)) {
            reportError("SendTo failed: " + std::to_string(error));
        }
    }

    return sent;
}

ssize_t UDPConnection::receive(byte* buffer, size_t size) {
    if (socket_ == INVALID_SOCKET_FD) {
        return -1;
    }

    ssize_t received = ::recv(socket_, reinterpret_cast<char*>(buffer), size, 0);

    if (received < 0) {
        int error = platform::get_last_error();
        if (!platform::is_would_block(error)) {
            reportError("Receive failed: " + std::to_string(error));
        }
    }

    return received;
}

ssize_t UDPConnection::receiveFrom(byte* buffer, size_t size, NetworkAddress& source) {
    if (socket_ == INVALID_SOCKET_FD) {
        return -1;
    }

    struct sockaddr_in src_addr;
    socklen_t src_len = sizeof(src_addr);

    ssize_t received = recvfrom(socket_, reinterpret_cast<char*>(buffer), size, 0,
                                reinterpret_cast<struct sockaddr*>(&src_addr), &src_len);

    if (received < 0) {
        int error = platform::get_last_error();
        if (!platform::is_would_block(error)) {
            reportError("ReceiveFrom failed: " + std::to_string(error));
        }
    } else {
        // Get source address
        char src_ip[INET_ADDRSTRLEN];
        inet_ntop(AF_INET, &src_addr.sin_addr, src_ip, INET_ADDRSTRLEN);
        source = NetworkAddress(src_ip, ntohs(src_addr.sin_port));
    }

    return received;
}

} // namespace core
} // namespace switchspy
