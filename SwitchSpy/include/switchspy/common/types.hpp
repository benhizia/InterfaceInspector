#pragma once

#include <cstdint>
#include <string>
#include <vector>
#include <memory>
#include <chrono>
#include <functional>

namespace switchspy {

// Type aliases
using byte = uint8_t;
using byte_vector = std::vector<byte>;
using timestamp_t = std::chrono::system_clock::time_point;
using duration_t = std::chrono::milliseconds;

// Connection types
enum class ConnectionType : uint8_t {
    TCP = 0,
    UDP = 1,
    Multicast = 2
};

// Connection mode
enum class ConnectionMode : uint8_t {
    Relay = 0,      // Real-time relay
    Record = 1,     // Relay + recording
    Replay = 2      // Replay from file
};

// Visualization mode
enum class VisualizationMode : uint8_t {
    Realtime = 0,   // Immediate visualization (may impact performance)
    Deferred = 1    // Deferred visualization (better performance)
};

// Packet direction
enum class PacketDirection : uint8_t {
    AtoB = 0,      // From source to destination
    BtoA = 1       // From destination to source
};

// Connection state
enum class ConnectionState : uint8_t {
    Disconnected = 0,
    Connecting = 1,
    Connected = 2,
    Listening = 3,
    Error = 4
};

// Network address
struct NetworkAddress {
    std::string host;
    uint16_t port;

    NetworkAddress() : port(0) {}
    NetworkAddress(const std::string& h, uint16_t p) : host(h), port(p) {}

    std::string toString() const {
        return host + ":" + std::to_string(port);
    }
};

// Packet information
struct PacketInfo {
    timestamp_t timestamp;
    PacketDirection direction;
    size_t size;
    byte_vector data;

    PacketInfo() : size(0) {}
};

// Connection statistics
struct ConnectionStats {
    uint64_t packets_sent_a_to_b = 0;
    uint64_t packets_sent_b_to_a = 0;
    uint64_t bytes_sent_a_to_b = 0;
    uint64_t bytes_sent_b_to_a = 0;
    uint64_t total_packets = 0;
    uint64_t total_bytes = 0;
    timestamp_t start_time;
    duration_t uptime;

    void update(PacketDirection dir, size_t bytes) {
        if (dir == PacketDirection::AtoB) {
            packets_sent_a_to_b++;
            bytes_sent_a_to_b += bytes;
        } else {
            packets_sent_b_to_a++;
            bytes_sent_b_to_a += bytes;
        }
        total_packets++;
        total_bytes += bytes;
    }
};

// Callbacks
using PacketCallback = std::function<void(const PacketInfo&)>;
using ErrorCallback = std::function<void(const std::string&)>;
using StateChangeCallback = std::function<void(ConnectionState, ConnectionState)>;

} // namespace switchspy
