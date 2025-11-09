#include "shared.hpp"
#include <iostream>
#include <thread>
#include <chrono>
#include <random>
#include <cmath>

// Platform-specific headers
#ifdef _WIN32
    #include <winsock2.h>
    #include <ws2tcpip.h>
    #pragma comment(lib, "ws2_32.lib")
    using ssize_t = int;
#else
    #include <sys/socket.h>
    #include <netinet/in.h>
    #include <arpa/inet.h>
    #include <unistd.h>
    #define SOCKET int
    #define INVALID_SOCKET -1
    #define SOCKET_ERROR -1
    #define closesocket close
#endif

int main(int argc, char** argv) {
    std::cout << "===== ROBOT TELEMETRY SENDER - Complex Structs Test =====" << std::endl;

    // Parse arguments
    const char* host = "127.0.0.1";
    int port = 8006;
    int delay_ms = 500;  // 2 telemetry updates per second

    if (argc > 1) host = argv[1];
    if (argc > 2) port = std::atoi(argv[2]);
    if (argc > 3) delay_ms = std::atoi(argv[3]);

    std::cout << "Configuration:" << std::endl;
    std::cout << "  Target: " << host << ":" << port << std::endl;
    std::cout << "  Update rate: " << (1000.0 / delay_ms) << " updates/second" << std::endl;
    std::cout << std::endl;

#ifdef _WIN32
    WSADATA wsa;
    if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0) {
        std::cerr << "WSAStartup failed" << std::endl;
        return 1;
    }
#endif

    // Create TCP socket
    SOCKET sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (sock == INVALID_SOCKET) {
        std::cerr << "Failed to create socket" << std::endl;
#ifdef _WIN32
        WSACleanup();
#endif
        return 1;
    }

    // Connect to SwitchSpy relay
    sockaddr_in server_addr{};
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port);
    inet_pton(AF_INET, host, &server_addr.sin_addr);

    if (connect(sock, (sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
        std::cerr << "Connection failed" << std::endl;
        closesocket(sock);
#ifdef _WIN32
        WSACleanup();
#endif
        return 1;
    }

    std::cout << "Connected! Sending robot telemetry..." << std::endl;
    std::cout << std::endl;

    // Random number generation
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<int32_t> pos_dist(-5000, 5000);  // ±5000mm = ±5m
    std::uniform_int_distribution<int16_t> angle_dist(-1800, 1800);  // ±180°
    std::uniform_int_distribution<uint8_t> battery_dist(20, 100);
    std::uniform_int_distribution<uint8_t> color_dist(0, 255);

    uint64_t sequence = 0;
    uint32_t robot_id = 42;  // Robot #42
    double time_sec = 0.0;

    while (true) {
        RobotTelemetry telemetry{};
        telemetry.robot_id = robot_id;
        telemetry.sequence = ++sequence;

        auto now = std::chrono::system_clock::now();
        telemetry.timestamp_ms = std::chrono::duration_cast<std::chrono::milliseconds>(
            now.time_since_epoch()).count();

        // Simulate circular motion
        time_sec += delay_ms / 1000.0;
        double radius = 3000.0;  // 3 meters
        telemetry.position.x_mm = static_cast<int32_t>(radius * std::cos(time_sec * 0.5));
        telemetry.position.y_mm = static_cast<int32_t>(radius * std::sin(time_sec * 0.5));
        telemetry.position.z_mm = 1000 + static_cast<int32_t>(500 * std::sin(time_sec));  // 1m ± 0.5m

        // Quaternion for orientation (normalized)
        double angle = time_sec * 0.3;
        telemetry.orientation.w_e4 = static_cast<int16_t>(10000 * std::cos(angle / 2));
        telemetry.orientation.x_e4 = 0;
        telemetry.orientation.y_e4 = 0;
        telemetry.orientation.z_e4 = static_cast<int16_t>(10000 * std::sin(angle / 2));

        // Velocity (derivative of position, scaled)
        telemetry.velocity.x_mm = static_cast<int32_t>(-radius * 0.5 * std::sin(time_sec * 0.5));
        telemetry.velocity.y_mm = static_cast<int32_t>(radius * 0.5 * std::cos(time_sec * 0.5));
        telemetry.velocity.z_mm = static_cast<int32_t>(500 * std::cos(time_sec));

        // Status color (changes based on state)
        if (telemetry.battery_percent < 25) {
            telemetry.status_color = {255, 0, 0, 255};  // Red (low battery)
        } else if (sequence % 20 == 0) {
            telemetry.status_color = {255, 165, 0, 255};  // Orange (task change)
        } else {
            telemetry.status_color = {0, 255, 0, 255};  // Green (normal)
        }

        // Sensor data
        telemetry.sensors.temperature_c100 = 2000 + static_cast<int16_t>(500 * std::sin(time_sec * 0.1));  // 20-25°C
        telemetry.sensors.pressure_pa = 101325;  // Standard atmospheric pressure
        telemetry.sensors.humidity_p10 = 450 + static_cast<uint16_t>(100 * std::sin(time_sec * 0.2));  // 35-55%

        // Battery (slowly drains)
        telemetry.battery_percent = 100 - static_cast<uint8_t>(sequence / 10) % 100;

        // Error flags
        telemetry.error_flags = 0;
        if (telemetry.battery_percent < 25) {
            telemetry.error_flags |= 0x02;  // Battery low
        }
        if (sequence % 50 == 0) {
            telemetry.error_flags |= 0x04;  // Simulated sensor fault
        }

        // Task ID (changes every 10 seconds)
        telemetry.task_id = static_cast<uint16_t>((sequence / 20) % 5);

        // Joint angles (simulated 6-axis robot)
        for (int i = 0; i < 6; ++i) {
            double joint_time = time_sec + i * 0.5;
            telemetry.joint_angles[i] = static_cast<int16_t>(900 * std::sin(joint_time * 0.3 + i));  // ±90°
        }

        // Tool state (cycles: idle → gripping → releasing)
        telemetry.tool_state = static_cast<uint8_t>((sequence / 5) % 3);

        for (int i = 0; i < 3; ++i) {
            telemetry.padding[i] = 0;
        }

        // Send telemetry
        ssize_t sent = send(sock, (char*)&telemetry, sizeof(telemetry), 0);
        if (sent != sizeof(telemetry)) {
            std::cerr << "Send failed or incomplete" << std::endl;
            break;
        }

        // Print status
        std::cout << "Seq: " << sequence
                  << " | Robot ID: " << robot_id
                  << " | Pos: (" << telemetry.position.x_mm << ", "
                  << telemetry.position.y_mm << ", " << telemetry.position.z_mm << ") mm"
                  << " | Battery: " << (int)telemetry.battery_percent << "%"
                  << " | Task: " << telemetry.task_id
                  << " | Tool: " << (int)telemetry.tool_state;

        if (telemetry.error_flags) {
            std::cout << " [ERRORS: 0x" << std::hex << (int)telemetry.error_flags << std::dec << "]";
        }

        std::cout << std::endl;

        std::this_thread::sleep_for(std::chrono::milliseconds(delay_ms));
    }

    closesocket(sock);
#ifdef _WIN32
    WSACleanup();
#endif

    return 0;
}
