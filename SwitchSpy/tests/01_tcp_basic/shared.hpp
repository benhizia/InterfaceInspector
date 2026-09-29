#pragma once
#include <cstdint>

/// Simple message structure for TCP testing
struct SimpleMessage {
    /// Message sequence number
    uint32_t sequence_number;

    /// Message timestamp in milliseconds
    uint64_t timestamp_ms;

    /// Temperature reading in Celsius
    float temperature;

    /// Status flags (0=OK, 1=Warning, 2=Error)
    uint8_t status;

    /// Padding for alignment
    uint8_t padding[3];
};
