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

// Multicast group configuration
const char* MULTICAST_GROUP = "239.255.0.1";
const int MULTICAST_PORT = 8003;

int main(int argc, char** argv) {
    std::cout << "===== MULTICAST RECEIVER - Passive Listener =====" << std::endl;

    // Parse arguments
    const char* group = MULTICAST_GROUP;
    int port = MULTICAST_PORT;

    if (argc > 1) group = argv[1];
    if (argc > 2) port = std::atoi(argv[2]);

    std::cout << "Configuration:" << std::endl;
    std::cout << "  Multicast group: " << group << ":" << port << std::endl;
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

    // Allow multiple sockets to bind to the same port (for multiple receivers)
    int reuse = 1;
    if (setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, (char*)&reuse, sizeof(reuse)) < 0) {
        std::cerr << "Failed to set SO_REUSEADDR" << std::endl;
        closesocket(sock);
#ifdef _WIN32
        WSACleanup();
#endif
        return 1;
    }

    // Bind to the multicast port
    sockaddr_in local_addr{};
    local_addr.sin_family = AF_INET;
    local_addr.sin_port = htons(port);
    local_addr.sin_addr.s_addr = INADDR_ANY;

    if (bind(sock, (sockaddr*)&local_addr, sizeof(local_addr)) < 0) {
        std::cerr << "Bind failed" << std::endl;
        closesocket(sock);
#ifdef _WIN32
        WSACleanup();
#endif
        return 1;
    }

    // Join multicast group
    ip_mreq mreq{};
    inet_pton(AF_INET, group, &mreq.imr_multiaddr);
    mreq.imr_interface.s_addr = INADDR_ANY;

    if (setsockopt(sock, IPPROTO_IP, IP_ADD_MEMBERSHIP, (char*)&mreq, sizeof(mreq)) < 0) {
        std::cerr << "Failed to join multicast group" << std::endl;
        closesocket(sock);
#ifdef _WIN32
        WSACleanup();
#endif
        return 1;
    }

    std::cout << "Joined multicast group successfully!" << std::endl;
    std::cout << "Listening for broadcasts..." << std::endl;
    std::cout << std::endl;

    uint64_t expected_seq = 1;
    uint64_t total_received = 0;
    uint64_t total_dropped = 0;
    uint64_t out_of_order = 0;

    while (true) {
        TelemetryBroadcast telemetry{};
        sockaddr_in sender_addr{};
        socklen_t sender_len = sizeof(sender_addr);

        ssize_t received = recvfrom(sock, (char*)&telemetry, sizeof(telemetry), 0,
                                     (sockaddr*)&sender_addr, &sender_len);

        if (received < 0) {
            std::cerr << "Receive failed" << std::endl;
            continue;
        }

        if (received != sizeof(TelemetryBroadcast)) {
            std::cerr << "Received incomplete data: " << received << " bytes" << std::endl;
            continue;
        }

        total_received++;

        // Check sequence
        if (telemetry.sequence > expected_seq) {
            uint64_t gap = telemetry.sequence - expected_seq;
            total_dropped += gap;
            std::cerr << ">>> DROPPED " << gap << " broadcasts! (expected "
                      << expected_seq << ", got " << telemetry.sequence << ")" << std::endl;
            expected_seq = telemetry.sequence + 1;
        } else if (telemetry.sequence < expected_seq) {
            out_of_order++;
            std::cerr << ">>> OUT OF ORDER: expected " << expected_seq
                      << ", got " << telemetry.sequence << std::endl;
        } else {
            expected_seq++;
        }

        // Print every 10th broadcast
        if (telemetry.sequence % 10 == 0) {
            double lat = telemetry.latitude_e6 / 1e6;
            double lon = telemetry.longitude_e6 / 1e6;
            double drop_rate = (total_dropped * 100.0) / (total_received + total_dropped);

            std::cout << "Seq: " << telemetry.sequence
                      << " | Recv: " << total_received
                      << " | Dropped: " << total_dropped
                      << " (" << static_cast<int>(drop_rate) << "%)"
                      << " | GPS: (" << lat << ", " << lon << ")"
                      << " | Alt: " << telemetry.altitude_m << "m"
                      << " | Sats: " << (int)telemetry.satellite_count;

            if (telemetry.status_flags & 0x04) {
                std::cout << " [EMERGENCY]";
            } else if (telemetry.status_flags & 0x02) {
                std::cout << " [LOW BATT]";
            }

            std::cout << std::endl;
        }
    }

    // Leave multicast group (unreachable, but good practice)
    setsockopt(sock, IPPROTO_IP, IP_DROP_MEMBERSHIP, (char*)&mreq, sizeof(mreq));
    closesocket(sock);
#ifdef _WIN32
    WSACleanup();
#endif

    return 0;
}
