#include "shared.hpp"
#include <iostream>
#include <cstring>
#include <chrono>
#include <thread>

#ifdef _WIN32
    #include <winsock2.h>
    #include <ws2tcpip.h>
    #pragma comment(lib, "ws2_32.lib")
    using socket_t = SOCKET;
    #define INVALID_SOCKET_FD INVALID_SOCKET
    #define close_socket closesocket
#else
    #include <sys/socket.h>
    #include <netinet/in.h>
    #include <arpa/inet.h>
    #include <unistd.h>
    using socket_t = int;
    #define INVALID_SOCKET_FD -1
    #define close_socket close
#endif

int main(int argc, char** argv) {
    std::cout << "===== SLOW RECEIVER - Rate Mismatch Test =====" << std::endl;
    std::cout << "This receiver processes at LOW SPEED to create" << std::endl;
    std::cout << "a rate mismatch and test buffer overflow handling." << std::endl;
    std::cout << "===============================================" << std::endl;
    std::cout << std::endl;

    int port = 9005;
    int process_delay_ms = 20;  // SLOW: 50 packets/second (2x slower than sender)

    if (argc > 1) port = std::atoi(argv[1]);
    if (argc > 2) process_delay_ms = std::atoi(argv[2]);

    std::cout << "Configuration:" << std::endl;
    std::cout << "  Listen port: " << port << std::endl;
    std::cout << "  Process rate: " << 1000/process_delay_ms << " packets/second" << std::endl;
    std::cout << "  Processing delay: " << process_delay_ms << "ms" << std::endl;
    std::cout << std::endl;

#ifdef _WIN32
    WSADATA wsaData;
    WSAStartup(MAKEWORD(2, 2), &wsaData);
#endif

    socket_t listen_sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (listen_sock == INVALID_SOCKET_FD) {
        std::cerr << "Failed to create socket" << std::endl;
        return 1;
    }

    struct sockaddr_in addr;
    std::memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    addr.sin_addr.s_addr = INADDR_ANY;

    int reuse = 1;
    setsockopt(listen_sock, SOL_SOCKET, SO_REUSEADDR, (char*)&reuse, sizeof(reuse));

    if (bind(listen_sock, (struct sockaddr*)&addr, sizeof(addr)) < 0) {
        std::cerr << "Bind failed" << std::endl;
        close_socket(listen_sock);
        return 1;
    }

    if (listen(listen_sock, 1) < 0) {
        std::cerr << "Listen failed" << std::endl;
        close_socket(listen_sock);
        return 1;
    }

    std::cout << "Listening..." << std::endl;

    socket_t client_sock = accept(listen_sock, nullptr, nullptr);
    if (client_sock == INVALID_SOCKET_FD) {
        std::cerr << "Accept failed" << std::endl;
        close_socket(listen_sock);
        return 1;
    }

    std::cout << "Connected! Starting slow processing..." << std::endl;
    std::cout << "NOTE: Expect to see gaps in sequence numbers due to buffer drops" << std::endl;
    std::cout << std::endl;

    uint64_t expected_seq = 0;
    uint64_t total_received = 0;
    uint64_t total_dropped = 0;
    auto start_time = std::chrono::steady_clock::now();

    while (true) {
        SensorData data;
        ssize_t received = recv(client_sock, (char*)&data, sizeof(data), 0);

        if (received == 0) {
            std::cout << "Connection closed" << std::endl;
            break;
        }

        if (received < 0) {
            std::cerr << "Receive failed" << std::endl;
            break;
        }

        if (received != sizeof(data)) {
            std::cerr << "Partial message: " << received << " bytes" << std::endl;
            continue;
        }

        total_received++;

        // Detect dropped packets
        if (data.sequence != expected_seq) {
            uint64_t gap = data.sequence - expected_seq;
            total_dropped += gap;
            std::cerr << ">>> DROPPED " << gap << " packets! (expected " << expected_seq
                      << ", got " << data.sequence << ")" << std::endl;
            expected_seq = data.sequence + 1;
        } else {
            expected_seq++;
        }

        // Print status every 50 packets
        if (total_received % 50 == 0) {
            auto now = std::chrono::steady_clock::now();
            auto elapsed = std::chrono::duration_cast<std::chrono::seconds>(now - start_time).count();
            double rate = total_received / std::max(1.0, (double)elapsed);
            double drop_rate = (total_dropped * 100.0) / (total_received + total_dropped);

            std::cout << "Seq: " << data.sequence
                      << " | Recv: " << total_received
                      << " | Dropped: " << total_dropped << " (" << (int)drop_rate << "%)"
                      << " | Rate: " << (int)rate << " pkt/s"
                      << " | Temp: " << data.temperature_c100/100.0 << "°C"
                      << std::endl;
        }

        // Simulate slow processing
        std::this_thread::sleep_for(std::chrono::milliseconds(process_delay_ms));
    }

    close_socket(client_sock);
    close_socket(listen_sock);

#ifdef _WIN32
    WSACleanup();
#endif

    std::cout << std::endl;
    std::cout << "=== Final Statistics ===" << std::endl;
    std::cout << "Total received: " << total_received << std::endl;
    std::cout << "Total dropped: " << total_dropped << std::endl;
    std::cout << "Drop rate: " << (total_dropped * 100.0) / (total_received + total_dropped) << "%" << std::endl;

    return 0;
}
