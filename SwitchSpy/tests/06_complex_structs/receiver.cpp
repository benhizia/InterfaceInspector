#include "shared.hpp"
#include <iostream>
#include <cstring>

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
    std::cout << "===== ROBOT TELEMETRY RECEIVER - Complex Structs Test =====" << std::endl;

    // Parse arguments
    int port = 9006;
    if (argc > 1) port = std::atoi(argv[1]);

    std::cout << "Configuration:" << std::endl;
    std::cout << "  Listen port: " << port << std::endl;
    std::cout << std::endl;

#ifdef _WIN32
    WSADATA wsa;
    if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0) {
        std::cerr << "WSAStartup failed" << std::endl;
        return 1;
    }
#endif

    // Create TCP socket
    SOCKET listen_sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (listen_sock == INVALID_SOCKET) {
        std::cerr << "Failed to create socket" << std::endl;
#ifdef _WIN32
        WSACleanup();
#endif
        return 1;
    }

    // Allow port reuse
    int reuse = 1;
    setsockopt(listen_sock, SOL_SOCKET, SO_REUSEADDR, (char*)&reuse, sizeof(reuse));

    // Bind
    sockaddr_in local_addr{};
    local_addr.sin_family = AF_INET;
    local_addr.sin_port = htons(port);
    local_addr.sin_addr.s_addr = INADDR_ANY;

    if (bind(listen_sock, (sockaddr*)&local_addr, sizeof(local_addr)) < 0) {
        std::cerr << "Bind failed" << std::endl;
        closesocket(listen_sock);
#ifdef _WIN32
        WSACleanup();
#endif
        return 1;
    }

    // Listen
    if (listen(listen_sock, 1) < 0) {
        std::cerr << "Listen failed" << std::endl;
        closesocket(listen_sock);
#ifdef _WIN32
        WSACleanup();
#endif
        return 1;
    }

    std::cout << "Listening for robot telemetry..." << std::endl;

    // Accept connection
    sockaddr_in client_addr{};
    socklen_t client_len = sizeof(client_addr);
    SOCKET client_sock = accept(listen_sock, (sockaddr*)&client_addr, &client_len);

    if (client_sock == INVALID_SOCKET) {
        std::cerr << "Accept failed" << std::endl;
        closesocket(listen_sock);
#ifdef _WIN32
        WSACleanup();
#endif
        return 1;
    }

    char client_ip[INET_ADDRSTRLEN];
    inet_ntop(AF_INET, &client_addr.sin_addr, client_ip, sizeof(client_ip));
    std::cout << "Connected from: " << client_ip << ":" << ntohs(client_addr.sin_port) << std::endl;
    std::cout << std::endl;

    uint64_t expected_seq = 1;
    uint64_t total_received = 0;
    uint64_t total_dropped = 0;

    const char* tool_states[] = {"IDLE", "GRIPPING", "RELEASING"};

    while (true) {
        RobotTelemetry telemetry{};
        size_t total_read = 0;

        // Read full telemetry (may require multiple recv calls)
        while (total_read < sizeof(RobotTelemetry)) {
            ssize_t received = recv(client_sock, (char*)&telemetry + total_read,
                                    sizeof(RobotTelemetry) - total_read, 0);

            if (received <= 0) {
                std::cout << "Connection closed or error" << std::endl;
                goto cleanup;
            }

            total_read += received;
        }

        total_received++;

        // Check sequence
        if (telemetry.sequence > expected_seq) {
            uint64_t gap = telemetry.sequence - expected_seq;
            total_dropped += gap;
            std::cerr << ">>> DROPPED " << gap << " messages! (expected "
                      << expected_seq << ", got " << telemetry.sequence << ")" << std::endl;
            expected_seq = telemetry.sequence + 1;
        } else if (telemetry.sequence < expected_seq) {
            std::cerr << ">>> OUT OF ORDER: expected " << expected_seq
                      << ", got " << telemetry.sequence << std::endl;
        } else {
            expected_seq++;
        }

        double drop_rate = (total_dropped * 100.0) / (total_received + total_dropped);

        // Parse and display
        std::cout << "Seq: " << telemetry.sequence
                  << " | Recv: " << total_received
                  << " | Dropped: " << total_dropped
                  << " (" << static_cast<int>(drop_rate) << "%)"
                  << std::endl;

        std::cout << "  Robot ID: " << telemetry.robot_id
                  << " | Battery: " << (int)telemetry.battery_percent << "%"
                  << " | Task: " << telemetry.task_id
                  << std::endl;

        std::cout << "  Position: ("
                  << telemetry.position.x_mm << ", "
                  << telemetry.position.y_mm << ", "
                  << telemetry.position.z_mm << ") mm"
                  << std::endl;

        std::cout << "  Orientation (quaternion): ("
                  << telemetry.orientation.w_e4 / 10000.0 << ", "
                  << telemetry.orientation.x_e4 / 10000.0 << ", "
                  << telemetry.orientation.y_e4 / 10000.0 << ", "
                  << telemetry.orientation.z_e4 / 10000.0 << ")"
                  << std::endl;

        std::cout << "  Velocity: ("
                  << telemetry.velocity.x_mm << ", "
                  << telemetry.velocity.y_mm << ", "
                  << telemetry.velocity.z_mm << ") mm/s"
                  << std::endl;

        std::cout << "  Status Color: RGB("
                  << (int)telemetry.status_color.r << ", "
                  << (int)telemetry.status_color.g << ", "
                  << (int)telemetry.status_color.b << ", "
                  << (int)telemetry.status_color.a << ")"
                  << std::endl;

        std::cout << "  Sensors: Temp="
                  << (telemetry.sensors.temperature_c100 / 100.0) << "°C, "
                  << "Pressure=" << telemetry.sensors.pressure_pa << " Pa, "
                  << "Humidity=" << (telemetry.sensors.humidity_p10 / 10.0) << "%"
                  << std::endl;

        std::cout << "  Joint Angles: [";
        for (int i = 0; i < 6; ++i) {
            std::cout << (telemetry.joint_angles[i] / 10.0) << "°";
            if (i < 5) std::cout << ", ";
        }
        std::cout << "]" << std::endl;

        const char* tool_state_str = (telemetry.tool_state < 3) ?
                                      tool_states[telemetry.tool_state] : "UNKNOWN";
        std::cout << "  Tool: " << tool_state_str;

        if (telemetry.error_flags) {
            std::cout << " | ERRORS: ";
            if (telemetry.error_flags & 0x01) std::cout << "COLLISION ";
            if (telemetry.error_flags & 0x02) std::cout << "LOW_BATTERY ";
            if (telemetry.error_flags & 0x04) std::cout << "SENSOR_FAULT";
        }

        std::cout << std::endl << std::endl;
    }

cleanup:
    closesocket(client_sock);
    closesocket(listen_sock);
#ifdef _WIN32
    WSACleanup();
#endif

    std::cout << std::endl;
    std::cout << "===== FINAL REPORT =====" << std::endl;
    std::cout << "Total received: " << total_received << std::endl;
    std::cout << "Total dropped: " << total_dropped << std::endl;

    return 0;
}
