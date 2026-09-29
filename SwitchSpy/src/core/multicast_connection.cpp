#include "switchspy/core/multicast_connection.hpp"
#include <cstring>

namespace switchspy {
namespace core {

MulticastConnection::MulticastConnection()
    : NetworkConnection(ConnectionType::Multicast)
    , joined_(false)
{
}

MulticastConnection::~MulticastConnection() {
    leaveGroup();
}

bool MulticastConnection::joinGroup(const std::string& group, uint16_t port,
                                    const std::string& interface_addr) {
    std::lock_guard<std::mutex> lock(mutex_);

    if (!createSocket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) {
        return false;
    }

    multicast_group_ = group;
    interface_addr_ = interface_addr;
    local_address_ = NetworkAddress(group, port);

    // Allow multiple sockets to bind to the same port
    int reuse = 1;
    if (setsockopt(socket_, SOL_SOCKET, SO_REUSEADDR,
                   reinterpret_cast<const char*>(&reuse), sizeof(reuse)) < 0) {
        reportError("Failed to set SO_REUSEADDR");
        close();
        return false;
    }

#ifdef SO_REUSEPORT
    // Enable port reuse on platforms that support it
    if (setsockopt(socket_, SOL_SOCKET, SO_REUSEPORT,
                   reinterpret_cast<const char*>(&reuse), sizeof(reuse)) < 0) {
        // Non-fatal on some platforms
    }
#endif

    // Bind to port (use INADDR_ANY for multicast)
    struct sockaddr_in addr;
    std::memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    addr.sin_addr.s_addr = INADDR_ANY; // Bind to any interface

    if (bind(socket_, reinterpret_cast<struct sockaddr*>(&addr), sizeof(addr)) < 0) {
        reportError("Bind failed: " + std::to_string(platform::get_last_error()));
        close();
        return false;
    }

    // Join multicast group
    struct ip_mreq mreq;
    std::memset(&mreq, 0, sizeof(mreq));

    if (inet_pton(AF_INET, group.c_str(), &mreq.imr_multiaddr) <= 0) {
        reportError("Invalid multicast address: " + group);
        close();
        return false;
    }

    if (interface_addr.empty()) {
        mreq.imr_interface.s_addr = INADDR_ANY;
    } else {
        if (inet_pton(AF_INET, interface_addr.c_str(), &mreq.imr_interface) <= 0) {
            reportError("Invalid interface address: " + interface_addr);
            close();
            return false;
        }
    }

    if (setsockopt(socket_, IPPROTO_IP, IP_ADD_MEMBERSHIP,
                   reinterpret_cast<const char*>(&mreq), sizeof(mreq)) < 0) {
        reportError("Failed to join multicast group: " + std::to_string(platform::get_last_error()));
        close();
        return false;
    }

    joined_ = true;
    setState(ConnectionState::Connected);
    return true;
}

void MulticastConnection::leaveGroup() {
    std::lock_guard<std::mutex> lock(mutex_);

    if (joined_ && socket_ != INVALID_SOCKET_FD) {
        struct ip_mreq mreq;
        std::memset(&mreq, 0, sizeof(mreq));

        inet_pton(AF_INET, multicast_group_.c_str(), &mreq.imr_multiaddr);

        if (interface_addr_.empty()) {
            mreq.imr_interface.s_addr = INADDR_ANY;
        } else {
            inet_pton(AF_INET, interface_addr_.c_str(), &mreq.imr_interface);
        }

        setsockopt(socket_, IPPROTO_IP, IP_DROP_MEMBERSHIP,
                   reinterpret_cast<const char*>(&mreq), sizeof(mreq));

        joined_ = false;
    }
}

bool MulticastConnection::connect(const NetworkAddress& remote) {
    std::lock_guard<std::mutex> lock(mutex_);

    if (socket_ == INVALID_SOCKET_FD) {
        if (!createSocket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) {
            return false;
        }
    }

    remote_address_ = remote;
    multicast_group_ = remote.host;

    // Set default TTL
    setTTL(1);

    setState(ConnectionState::Connected);
    return true;
}

bool MulticastConnection::listen(const NetworkAddress& local) {
    return joinGroup(local.host, local.port);
}

ssize_t MulticastConnection::send(const byte* data, size_t size) {
    if (socket_ == INVALID_SOCKET_FD) {
        return -1;
    }

    struct sockaddr_in addr;
    std::memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port = htons(remote_address_.port);

    if (inet_pton(AF_INET, multicast_group_.c_str(), &addr.sin_addr) <= 0) {
        reportError("Invalid multicast address");
        return -1;
    }

    ssize_t sent = sendto(socket_, reinterpret_cast<const char*>(data), size, 0,
                          reinterpret_cast<struct sockaddr*>(&addr), sizeof(addr));

    if (sent < 0) {
        int error = platform::get_last_error();
        if (!platform::is_would_block(error)) {
            reportError("Multicast send failed: " + std::to_string(error));
        }
    }

    return sent;
}

ssize_t MulticastConnection::receive(byte* buffer, size_t size) {
    if (socket_ == INVALID_SOCKET_FD) {
        return -1;
    }

    ssize_t received = ::recv(socket_, reinterpret_cast<char*>(buffer), size, 0);

    if (received < 0) {
        int error = platform::get_last_error();
        if (!platform::is_would_block(error)) {
            reportError("Multicast receive failed: " + std::to_string(error));
        }
    }

    return received;
}

ssize_t MulticastConnection::receiveFrom(byte* buffer, size_t size, NetworkAddress& source) {
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
            reportError("Multicast receiveFrom failed: " + std::to_string(error));
        }
    } else {
        char src_ip[INET_ADDRSTRLEN];
        inet_ntop(AF_INET, &src_addr.sin_addr, src_ip, INET_ADDRSTRLEN);
        source = NetworkAddress(src_ip, ntohs(src_addr.sin_port));
    }

    return received;
}

void MulticastConnection::setTTL(int ttl) {
    if (socket_ == INVALID_SOCKET_FD) {
        return;
    }

    unsigned char ttl_val = static_cast<unsigned char>(ttl);
    if (setsockopt(socket_, IPPROTO_IP, IP_MULTICAST_TTL,
                   reinterpret_cast<const char*>(&ttl_val), sizeof(ttl_val)) < 0) {
        reportError("Failed to set multicast TTL");
    }
}

void MulticastConnection::setLoopback(bool enable) {
    if (socket_ == INVALID_SOCKET_FD) {
        return;
    }

    unsigned char loop = enable ? 1 : 0;
    if (setsockopt(socket_, IPPROTO_IP, IP_MULTICAST_LOOP,
                   reinterpret_cast<const char*>(&loop), sizeof(loop)) < 0) {
        reportError("Failed to set multicast loopback");
    }
}

} // namespace core
} // namespace switchspy
