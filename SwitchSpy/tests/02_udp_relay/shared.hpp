#pragma once

#include <cstdint>

/// Command packet for remote device control
struct CommandPacket {
    /// Command identifier (1=Reboot, 2=Status, 3=Config, 4=Data)
    uint8_t command_id;

    /// Device address (target device)
    uint8_t device_addr;

    /// Payload length in bytes
    uint16_t payload_length;

    /// Command sequence number (for acknowledgment tracking)
    uint32_t sequence;

    /// Timestamp in milliseconds
    uint64_t timestamp_ms;

    /// Payload data (up to 64 bytes)
    uint8_t payload[64];

    /// CRC32 checksum
    uint32_t checksum;
};

static_assert(sizeof(CommandPacket) == 84, "CommandPacket size mismatch");
