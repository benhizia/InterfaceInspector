#pragma once
#include <cstdint>

/// Sensor data packet for rate mismatch testing
struct SensorData {
    /// Packet sequence number
    uint64_t sequence;

    /// Sensor ID
    uint32_t sensor_id;

    /// Temperature in Celsius * 100 (e.g., 2550 = 25.50°C)
    int16_t temperature_c100;

    /// Humidity in percent * 10 (e.g., 652 = 65.2%)
    uint16_t humidity_p10;

    /// Pressure in Pa
    uint32_t pressure_pa;

    /// Sensor status bitfield
    uint16_t status_flags;

    /// Battery level in percent
    uint8_t battery_percent;

    /// Reserved for alignment
    uint8_t reserved;
};
