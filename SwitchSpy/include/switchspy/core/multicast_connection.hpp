#pragma once

#include "switchspy/core/network_connection.hpp"

namespace switchspy {
namespace core {

/**
 * Multicast connection implementation
 * Supports joining multicast groups and sending/receiving multicast datagrams
 */
class MulticastConnection : public NetworkConnection {
public:
    MulticastConnection();
    ~MulticastConnection() override;

    /**
     * Join multicast group
     * @param group Multicast group address (e.g., "239.1.1.1")
     * @param port Port to receive on
     * @param interface_addr Local interface address (empty for default)
     * @return true if joined successfully
     */
    bool joinGroup(const std::string& group, uint16_t port,
                   const std::string& interface_addr = "");

    /**
     * Leave multicast group
     */
    void leaveGroup();

    /**
     * Connect to multicast group (for sending)
     */
    bool connect(const NetworkAddress& remote) override;

    /**
     * Listen on multicast group (same as joinGroup)
     */
    bool listen(const NetworkAddress& local) override;

    /**
     * Send multicast datagram
     */
    ssize_t send(const byte* data, size_t size) override;

    /**
     * Receive multicast datagram
     */
    ssize_t receive(byte* buffer, size_t size) override;

    /**
     * Receive with source information
     */
    ssize_t receiveFrom(byte* buffer, size_t size, NetworkAddress& source);

    /**
     * Set multicast TTL (time-to-live)
     * @param ttl TTL value (1 = local network, higher = wider reach)
     */
    void setTTL(int ttl);

    /**
     * Enable/disable multicast loopback
     * @param enable true to receive own multicast packets
     */
    void setLoopback(bool enable);

private:
    std::string multicast_group_;
    std::string interface_addr_;
    bool joined_;
};

} // namespace core
} // namespace switchspy
