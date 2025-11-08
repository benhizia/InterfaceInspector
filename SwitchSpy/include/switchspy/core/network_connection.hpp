#pragma once

#include "switchspy/common/types.hpp"
#include "switchspy/common/platform.hpp"
#include <atomic>
#include <mutex>
#include <string>

namespace switchspy {
namespace core {

/**
 * Base class for all network connection types
 * Provides common interface for TCP, UDP, and Multicast connections
 */
class NetworkConnection {
public:
    explicit NetworkConnection(ConnectionType type);
    virtual ~NetworkConnection();

    // Prevent copying
    NetworkConnection(const NetworkConnection&) = delete;
    NetworkConnection& operator=(const NetworkConnection&) = delete;

    // Allow moving
    NetworkConnection(NetworkConnection&&) noexcept = default;
    NetworkConnection& operator=(NetworkConnection&&) noexcept = default;

    /**
     * Connect to remote endpoint
     * @param remote Remote address to connect to
     * @return true if connection initiated successfully
     */
    virtual bool connect(const NetworkAddress& remote) = 0;

    /**
     * Start listening for incoming connections
     * @param local Local address to bind to
     * @return true if listening started successfully
     */
    virtual bool listen(const NetworkAddress& local) = 0;

    /**
     * Send data through the connection
     * @param data Pointer to data buffer
     * @param size Size of data to send
     * @return Number of bytes sent, or -1 on error
     */
    virtual ssize_t send(const byte* data, size_t size) = 0;

    /**
     * Receive data from the connection
     * @param buffer Buffer to store received data
     * @param size Maximum size to receive
     * @return Number of bytes received, or -1 on error
     */
    virtual ssize_t receive(byte* buffer, size_t size) = 0;

    /**
     * Close the connection
     */
    virtual void close();

    /**
     * Check if connection is active
     * @return true if connected
     */
    bool isConnected() const { return state_ == ConnectionState::Connected; }

    /**
     * Get current connection state
     */
    ConnectionState getState() const { return state_; }

    /**
     * Get connection type
     */
    ConnectionType getType() const { return type_; }

    /**
     * Get local address
     */
    const NetworkAddress& getLocalAddress() const { return local_address_; }

    /**
     * Get remote address
     */
    const NetworkAddress& getRemoteAddress() const { return remote_address_; }

    /**
     * Get socket file descriptor
     */
    socket_t getSocket() const { return socket_; }

    /**
     * Set state change callback
     */
    void setStateChangeCallback(StateChangeCallback callback) {
        state_change_callback_ = callback;
    }

    /**
     * Set error callback
     */
    void setErrorCallback(ErrorCallback callback) {
        error_callback_ = callback;
    }

protected:
    // Update connection state
    void setState(ConnectionState new_state);

    // Report error
    void reportError(const std::string& error);

    // Create socket
    bool createSocket(int domain, int type, int protocol);

    // Socket file descriptor
    socket_t socket_;

    // Connection type
    ConnectionType type_;

    // Connection state
    std::atomic<ConnectionState> state_;

    // Addresses
    NetworkAddress local_address_;
    NetworkAddress remote_address_;

    // Callbacks
    StateChangeCallback state_change_callback_;
    ErrorCallback error_callback_;

    // Mutex for thread-safe operations
    mutable std::mutex mutex_;
};

} // namespace core
} // namespace switchspy
