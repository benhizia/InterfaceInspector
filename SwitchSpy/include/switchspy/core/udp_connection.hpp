#pragma once

#include "switchspy/core/network_connection.hpp"

namespace switchspy {
namespace core {

/**
 * UDP connection implementation
 * Supports connectionless datagram communication
 */
class UDPConnection : public NetworkConnection {
public:
    UDPConnection();
    ~UDPConnection() override = default;

    /**
     * "Connect" to remote UDP endpoint (sets default destination)
     */
    bool connect(const NetworkAddress& remote) override;

    /**
     * Bind to local address for receiving
     */
    bool listen(const NetworkAddress& local) override;

    /**
     * Send datagram to connected remote or specified destination
     */
    ssize_t send(const byte* data, size_t size) override;

    /**
     * Send datagram to specific destination
     */
    ssize_t sendTo(const byte* data, size_t size, const NetworkAddress& dest);

    /**
     * Receive datagram
     */
    ssize_t receive(byte* buffer, size_t size) override;

    /**
     * Receive datagram and get source address
     */
    ssize_t receiveFrom(byte* buffer, size_t size, NetworkAddress& source);

private:
    bool bound_;
};

} // namespace core
} // namespace switchspy
