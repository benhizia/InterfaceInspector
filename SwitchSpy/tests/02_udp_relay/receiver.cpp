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

// Simple CRC32 implementation (must match sender)
uint32_t calculate_crc32(const uint8_t* data, size_t length) {
    uint32_t crc = 0xFFFFFFFF;
    for (size_t i = 0; i < length; ++i) {
        crc ^= data[i];
        for (int j = 0; j < 8; ++j) {
            crc = (crc >> 1) ^ (0xEDB88320 & -(crc & 1));
        }
    }
    return ~crc;
}

int main(int argc, char** argv) {
    std::cout << "===== UDP COMMAND RECEIVER =====" << std::endl;

    // Parse arguments
    int port = 9002;
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

    // Create UDP socket
    SOCKET sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (sock == INVALID_SOCKET) {
        std::cerr << "Failed to create socket" << std::endl;
#ifdef _WIN32
        WSACleanup();
#endif
        return 1;
    }

    // Bind to port
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

    std::cout << "Listening for UDP commands..." << std::endl;
    std::cout << std::endl;

    const char* command_names[] = {"UNKNOWN", "REBOOT", "STATUS", "CONFIG", "DATA"};

    uint32_t expected_seq = 1;
    uint64_t total_received = 0;
    uint64_t total_dropped = 0;
    uint64_t checksum_errors = 0;

    while (true) {
        CommandPacket cmd{};
        sockaddr_in sender_addr{};
        socklen_t sender_len = sizeof(sender_addr);

        ssize_t received = recvfrom(sock, (char*)&cmd, sizeof(cmd), 0,
                                     (sockaddr*)&sender_addr, &sender_len);

        if (received < 0) {
            std::cerr << "Receive failed" << std::endl;
            continue;
        }

        if (received != sizeof(CommandPacket)) {
            std::cerr << "Received incomplete packet: " << received << " bytes (expected "
                      << sizeof(CommandPacket) << ")" << std::endl;
            continue;
        }

        total_received++;

        // Verify checksum
        uint32_t expected_crc = calculate_crc32((uint8_t*)&cmd, sizeof(cmd) - sizeof(cmd.checksum));
        if (cmd.checksum != expected_crc) {
            checksum_errors++;
            std::cerr << ">>> CHECKSUM ERROR: expected 0x" << std::hex << expected_crc
                      << ", got 0x" << cmd.checksum << std::dec << std::endl;
            continue;  // Skip corrupted packet
        }

        // Check sequence (UDP can reorder or drop datagrams)
        if (cmd.sequence > expected_seq) {
            uint32_t gap = cmd.sequence - expected_seq;
            total_dropped += gap;
            std::cerr << ">>> DROPPED " << gap << " packets! (expected "
                      << expected_seq << ", got " << cmd.sequence << ")" << std::endl;
            expected_seq = cmd.sequence + 1;
        } else if (cmd.sequence < expected_seq) {
            std::cerr << ">>> OUT OF ORDER: expected " << expected_seq
                      << ", got " << cmd.sequence << std::endl;
        } else {
            expected_seq++;
        }

        // Print every 5th command
        if (cmd.sequence % 5 == 0) {
            double drop_rate = (total_dropped * 100.0) / (total_received + total_dropped);

            const char* cmd_name = (cmd.command_id <= 4) ? command_names[cmd.command_id] : "INVALID";

            std::cout << "Seq: " << cmd.sequence
                      << " | Recv: " << total_received
                      << " | Dropped: " << total_dropped
                      << " (" << static_cast<int>(drop_rate) << "%)"
                      << " | Cmd: " << cmd_name
                      << " | Device: " << (int)cmd.device_addr
                      << " | Payload: " << cmd.payload_length << " bytes";

            if (checksum_errors > 0) {
                std::cout << " | CRC Errors: " << checksum_errors;
            }

            std::cout << std::endl;
        }
    }

    closesocket(sock);
#ifdef _WIN32
    WSACleanup();
#endif

    return 0;
}
