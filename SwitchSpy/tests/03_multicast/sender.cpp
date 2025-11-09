#include "shared.hpp"
#include <iostream>
#include <thread>
#include <chrono>
#include <random>

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

// Multicast group configuration
const char* MULTICAST_GROUP = "239.255.0.1";
const int MULTICAST_PORT = 8003;

int main(int argc, char** argv) {
    std::cout << "===== MULTICAST SENDER - Broadcast Test =====" << std::endl;

    // Parse arguments
    const char* group = MULTICAST_GROUP;
    int port = MULTICAST_PORT;
    int delay_ms = 1000;  // 1 broadcast per second

    if (argc > 1) group = argv[1];
    if (argc > 2) port = std::atoi(argv[2]);
    if (argc > 3) delay_ms = std::atoi(argv[3]);

    std::cout << "Configuration:" << std::endl;
    std::cout << "  Multicast group: " << group << ":" << port << std::endl;
    std::cout << "  Broadcast rate: " << (1000.0 / delay_ms) << " broadcasts/second" << std::endl;
    std::cout << std::endl;

#ifdef _WIN32
    WSADATA wsa;
    if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0) {
        std::cerr << "WSAStartup failed" << std::endl;
        return 1;
    }
#endif

    // Create UDP socket
    SOCKET sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (sock == INVALID_SOCKET) {
        std::cerr << "Failed to create socket" << std::endl;
#ifdef _WIN32
        WSACleanup();
#endif
        return 1;
    }

    // Enable broadcast/multicast
    int enable = 1;
    if (setsockopt(sock, SOL_SOCKET, SO_BROADCAST, (char*)&enable, sizeof(enable)) < 0) {
        std::cerr << "Failed to enable broadcast" << std::endl;
        closesocket(sock);
#ifdef _WIN32
        WSACleanup();
#endif
        return 1;
    }

    // Set multicast TTL (time to live)
    unsigned char ttl = 32;
    if (setsockopt(sock, IPPROTO_IP, IP_MULTICAST_TTL, (char*)&ttl, sizeof(ttl)) < 0) {
        std::cerr << "Failed to set multicast TTL" << std::endl;
        closesocket(sock);
#ifdef _WIN32
        WSACleanup();
#endif
        return 1;
    }

    // Setup destination address
    sockaddr_in dest_addr{};
    dest_addr.sin_family = AF_INET;
    dest_addr.sin_port = htons(port);
    inet_pton(AF_INET, group, &dest_addr.sin_addr);

    std::cout << "Broadcasting telemetry data..." << std::endl;
    std::cout << std::endl;

    // Random number generation for simulated sensor data
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<int32_t> lat_dist(37000000, 38000000);  // ~37-38°N
    std::uniform_int_distribution<int32_t> lon_dist(-122500000, -121500000);  // ~122-121°W
    std::uniform_int_distribution<int16_t> alt_dist(0, 500);  // 0-500m
    std::uniform_int_distribution<uint16_t> speed_dist(0, 2000);  // 0-20 m/s
    std::uniform_int_distribution<uint16_t> heading_dist(0, 359);
    std::uniform_int_distribution<uint16_t> battery_dist(3000, 4200);  // 3.0-4.2V
    std::uniform_int_distribution<int8_t> signal_dist(-100, -50);  // -100 to -50 dBm
    std::uniform_int_distribution<uint8_t> sat_dist(4, 12);  // 4-12 satellites

    uint64_t sequence = 0;
    uint32_t source_id = 12345;  // Fixed source ID for this sender
    uint64_t total_sent = 0;
    auto start_time = std::chrono::steady_clock::now();

    while (true) {
        TelemetryBroadcast telemetry{};
        telemetry.source_id = source_id;
        telemetry.sequence = ++sequence;

        auto now = std::chrono::system_clock::now();
        telemetry.timestamp_ms = std::chrono::duration_cast<std::chrono::milliseconds>(
            now.time_since_epoch()).count();

        // Simulate GPS and sensor data
        telemetry.latitude_e6 = lat_dist(gen);
        telemetry.longitude_e6 = lon_dist(gen);
        telemetry.altitude_m = alt_dist(gen);
        telemetry.speed_cms = speed_dist(gen);
        telemetry.heading_deg = heading_dist(gen);
        telemetry.battery_mv = battery_dist(gen);
        telemetry.signal_dbm = signal_dist(gen);
        telemetry.satellite_count = sat_dist(gen);

        // Status flags
        telemetry.status_flags = 0x01;  // GPS fix
        if (telemetry.battery_mv < 3300) {
            telemetry.status_flags |= 0x02;  // Low battery
        }
        if (sequence % 50 == 0) {
            telemetry.status_flags |= 0x04;  // Emergency (every 50th broadcast)
        }

        telemetry.padding = 0;

        // Send to multicast group
        ssize_t sent = sendto(sock, (char*)&telemetry, sizeof(telemetry), 0,
                              (sockaddr*)&dest_addr, sizeof(dest_addr));

        if (sent < 0) {
            std::cerr << "Send failed" << std::endl;
            continue;
        }

        total_sent++;

        // Print status every 10 broadcasts
        if (sequence % 10 == 0) {
            auto elapsed = std::chrono::steady_clock::now() - start_time;
            double elapsed_sec = std::chrono::duration<double>(elapsed).count();
            double rate = total_sent / elapsed_sec;

            double lat = telemetry.latitude_e6 / 1e6;
            double lon = telemetry.longitude_e6 / 1e6;

            std::cout << "Seq: " << sequence
                      << " | Sent: " << total_sent
                      << " | Rate: " << static_cast<int>(rate) << " bcast/s"
                      << " | GPS: (" << lat << ", " << lon << ")"
                      << " | Sats: " << (int)telemetry.satellite_count
                      << " | Batt: " << telemetry.battery_mv << "mV";

            if (telemetry.status_flags & 0x04) {
                std::cout << " [EMERGENCY]";
            } else if (telemetry.status_flags & 0x02) {
                std::cout << " [LOW BATTERY]";
            }

            std::cout << std::endl;
        }

        std::this_thread::sleep_for(std::chrono::milliseconds(delay_ms));
    }

    closesocket(sock);
#ifdef _WIN32
    WSACleanup();
#endif

    return 0;
}
