#include "shared.hpp"
#include <iostream>
#include <thread>
#include <chrono>
#include <cstring>
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

// Simple CRC32 implementation
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
    std::cout << "===== UDP COMMAND SENDER =====" << std::endl;

    // Parse arguments
    const char* host = "127.0.0.1";
    int port = 8002;
    int delay_ms = 1000;  // 1 command per second

    if (argc > 1) host = argv[1];
    if (argc > 2) port = std::atoi(argv[2]);
    if (argc > 3) delay_ms = std::atoi(argv[3]);

    std::cout << "Configuration:" << std::endl;
    std::cout << "  Target: " << host << ":" << port << std::endl;
    std::cout << "  Send rate: " << (1000.0 / delay_ms) << " commands/second" << std::endl;
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

    // Setup destination address
    sockaddr_in dest_addr{};
    dest_addr.sin_family = AF_INET;
    dest_addr.sin_port = htons(port);
    inet_pton(AF_INET, host, &dest_addr.sin_addr);

    std::cout << "Sending commands via UDP..." << std::endl;
    std::cout << std::endl;

    // Random number generation for payload
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<uint8_t> payload_dist(0, 255);
    std::uniform_int_distribution<uint8_t> device_dist(1, 10);
    std::uniform_int_distribution<uint16_t> payload_len_dist(0, 64);

    uint32_t sequence = 0;
    uint64_t total_sent = 0;
    auto start_time = std::chrono::steady_clock::now();

    const char* command_names[] = {"UNKNOWN", "REBOOT", "STATUS", "CONFIG", "DATA"};

    while (true) {
        CommandPacket cmd{};

        // Cycle through command types
        cmd.command_id = (sequence % 4) + 1;  // 1-4
        cmd.device_addr = device_dist(gen);
        cmd.sequence = ++sequence;

        auto now = std::chrono::system_clock::now();
        cmd.timestamp_ms = std::chrono::duration_cast<std::chrono::milliseconds>(
            now.time_since_epoch()).count();

        // Generate random payload
        cmd.payload_length = payload_len_dist(gen);
        for (uint16_t i = 0; i < cmd.payload_length; ++i) {
            cmd.payload[i] = payload_dist(gen);
        }
        // Zero remaining payload
        for (uint16_t i = cmd.payload_length; i < 64; ++i) {
            cmd.payload[i] = 0;
        }

        // Calculate checksum (excluding checksum field itself)
        cmd.checksum = calculate_crc32((uint8_t*)&cmd, sizeof(cmd) - sizeof(cmd.checksum));

        // Send datagram
        ssize_t sent = sendto(sock, (char*)&cmd, sizeof(cmd), 0,
                              (sockaddr*)&dest_addr, sizeof(dest_addr));

        if (sent < 0) {
            std::cerr << "Send failed" << std::endl;
            continue;
        }

        total_sent++;

        // Print status every 5 commands
        if (sequence % 5 == 0) {
            auto elapsed = std::chrono::steady_clock::now() - start_time;
            double elapsed_sec = std::chrono::duration<double>(elapsed).count();
            double rate = total_sent / elapsed_sec;

            std::cout << "Seq: " << sequence
                      << " | Cmd: " << command_names[cmd.command_id]
                      << " | Device: " << (int)cmd.device_addr
                      << " | Payload: " << cmd.payload_length << " bytes"
                      << " | Rate: " << static_cast<int>(rate) << " cmd/s"
                      << " | CRC: 0x" << std::hex << cmd.checksum << std::dec
                      << std::endl;
        }

        std::this_thread::sleep_for(std::chrono::milliseconds(delay_ms));
    }

    closesocket(sock);
#ifdef _WIN32
    WSACleanup();
#endif

    return 0;
}
