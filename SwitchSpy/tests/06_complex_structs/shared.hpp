#pragma once

#include <cstdint>

/// 3D position vector
struct Vec3 {
    /// X coordinate in millimeters
    int32_t x_mm;

    /// Y coordinate in millimeters
    int32_t y_mm;

    /// Z coordinate in millimeters
    int32_t z_mm;
};

/// Quaternion rotation (w, x, y, z)
struct Quaternion {
    /// W component (scaled by 10000)
    int16_t w_e4;

    /// X component (scaled by 10000)
    int16_t x_e4;

    /// Y component (scaled by 10000)
    int16_t y_e4;

    /// Z component (scaled by 10000)
    int16_t z_e4;
};

/// RGB color
struct Color {
    /// Red channel (0-255)
    uint8_t r;

    /// Green channel (0-255)
    uint8_t g;

    /// Blue channel (0-255)
    uint8_t b;

    /// Alpha channel (0-255)
    uint8_t a;
};

/// Sensor data
struct SensorData {
    /// Temperature in hundredths of Celsius
    int16_t temperature_c100;

    /// Pressure in pascals
    uint32_t pressure_pa;

    /// Humidity in tenths of percent
    uint16_t humidity_p10;
};

/// Robot telemetry with nested structures
struct RobotTelemetry {
    /// Robot identifier
    uint32_t robot_id;

    /// Message sequence number
    uint64_t sequence;

    /// Timestamp in milliseconds
    uint64_t timestamp_ms;

    /// Robot position (nested struct)
    Vec3 position;

    /// Robot orientation (nested struct)
    Quaternion orientation;

    /// Velocity in mm/s (nested struct)
    Vec3 velocity;

    /// Current task color indicator (nested struct)
    Color status_color;

    /// Environmental sensors (nested struct)
    SensorData sensors;

    /// Battery percentage (0-100)
    uint8_t battery_percent;

    /// Error flags (bit 0: collision, bit 1: battery low, bit 2: sensor fault)
    uint8_t error_flags;

    /// Current task ID
    uint16_t task_id;

    /// Joint angles in decidegrees (6-axis robot)
    int16_t joint_angles[6];

    /// Tool state (0=idle, 1=gripping, 2=releasing)
    uint8_t tool_state;

    /// Padding for alignment
    uint8_t padding[3];
};

static_assert(sizeof(RobotTelemetry) == 96, "RobotTelemetry size mismatch");
