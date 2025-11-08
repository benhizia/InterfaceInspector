#pragma once

#include "switchspy/core/network_connection.hpp"

namespace switchspy {
namespace core {

/**
 * TCP connection implementation
 * Supports both client (connect) and server (listen/accept) modes
 */
class TCPConnection : public NetworkConnection {
public:
    TCPConnection();
    ~TCPConnection() override = default;

    /**
     * Connect to remote TCP endpoint
     */
    bool connect(const NetworkAddress& remote) override;

    /**
     * Start listening for TCP connections
     */
    bool listen(const NetworkAddress& local) override;

    /**
     * Accept incoming connection (call after listen)
     * @return New TCPConnection for the accepted client, or nullptr on error
     */
    std::unique_ptr<TCPConnection> accept();

    /**
     * Send data over TCP
     */
    ssize_t send(const byte* data, size_t size) override;

    /**
     * Receive data from TCP
     */
    ssize_t receive(byte* buffer, size_t size) override;

private:
    // Private constructor for accepted connections
    TCPConnection(socket_t accepted_socket, const NetworkAddress& remote);
};

} // namespace core
} // namespace switchspy
