#pragma once

#include "switchspy/common/types.hpp"
#include <string>

namespace switchspy {
namespace config {

/**
 * Port and address mapping configuration for a single connection
 */
struct PortMapping {
    std::string name;               // Connection name/identifier
    ConnectionType type;            // TCP, UDP, or Multicast

    // Listen configuration
    std::string listen_host;
    uint16_t listen_port;

    // Forward configuration
    std::string forward_host;
    uint16_t forward_port;

    // Multicast configuration
    std::string multicast_group;
    std::string multicast_interface;

    // Metadata file for struct visualization
    std::string metadata_file;

    // Mode configuration
    bool recording_enabled;
    std::string recording_file;

    PortMapping()
        : type(ConnectionType::TCP)
        , listen_port(0)
        , forward_port(0)
        , recording_enabled(false)
    {}

    bool isValid() const {
        if (type == ConnectionType::Multicast) {
            return !multicast_group.empty() && listen_port != 0;
        } else {
            return listen_port != 0 && !forward_host.empty() && forward_port != 0;
        }
    }
};

} // namespace config
} // namespace switchspy
