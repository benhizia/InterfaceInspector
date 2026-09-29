#include "shared.hpp"
#include <iostream>
#include <cstring>
#include <chrono>

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
    std::cout << "===== HIGH THROUGHPUT RECEIVER - Market Data Consumer =====" << std::endl;

    // Parse arguments
    int port = 9004;
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

    std::cout << "Listening for connections..." << std::endl;

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

    // Disable Nagle's algorithm
    int flag = 1;
    setsockopt(client_sock, IPPROTO_TCP, TCP_NODELAY, (char*)&flag, sizeof(flag));

    uint64_t expected_seq = 1;
    uint64_t total_received = 0;
    uint64_t total_dropped = 0;
    uint64_t trade_count = 0;
    uint64_t quote_count = 0;

    auto start_time = std::chrono::steady_clock::now();
    auto last_report = start_time;

    while (true) {
        MarketTick tick{};
        size_t total_read = 0;

        // Read full tick (may require multiple recv calls)
        while (total_read < sizeof(MarketTick)) {
            ssize_t received = recv(client_sock, (char*)&tick + total_read,
                                    sizeof(MarketTick) - total_read, 0);

            if (received <= 0) {
                std::cout << "Connection closed or error" << std::endl;
                goto cleanup;
            }

            total_read += received;
        }

        total_received++;

        // Check sequence
        if (tick.sequence > expected_seq) {
            uint64_t gap = tick.sequence - expected_seq;
            total_dropped += gap;
            std::cerr << ">>> DROPPED " << gap << " ticks! (expected "
                      << expected_seq << ", got " << tick.sequence << ")" << std::endl;
            expected_seq = tick.sequence + 1;
        } else if (tick.sequence < expected_seq) {
            std::cerr << ">>> OUT OF ORDER: expected " << expected_seq
                      << ", got " << tick.sequence << std::endl;
        } else {
            expected_seq++;
        }

        // Count trade vs quote
        if (tick.flags & 0x01) {
            trade_count++;
        } else if (tick.flags & 0x02) {
            quote_count++;
        }

        // Report every 1000 ticks
        if (tick.sequence % 1000 == 0) {
            auto now_report = std::chrono::steady_clock::now();
            double elapsed = std::chrono::duration<double>(now_report - last_report).count();
            double rate = 1000.0 / elapsed;
            double total_elapsed = std::chrono::duration<double>(now_report - start_time).count();
            double avg_rate = total_received / total_elapsed;
            double drop_rate = (total_dropped * 100.0) / (total_received + total_dropped);
            double throughput = (avg_rate * sizeof(MarketTick)) / (1024.0 * 1024.0);

            std::cout << "Seq: " << tick.sequence
                      << " | Recv: " << total_received
                      << " | Dropped: " << total_dropped
                      << " (" << static_cast<int>(drop_rate) << "%)"
                      << " | Rate: " << static_cast<int>(rate) << " ticks/s"
                      << " | Avg: " << static_cast<int>(avg_rate) << " ticks/s"
                      << " | Throughput: " << throughput << " MB/s"
                      << " | Trades: " << trade_count
                      << " | Quotes: " << quote_count
                      << std::endl;

            last_report = now_report;
        }
    }

cleanup:
    closesocket(client_sock);
    closesocket(listen_sock);
#ifdef _WIN32
    WSACleanup();
#endif

    // Final report
    double total_elapsed = std::chrono::duration<double>(
        std::chrono::steady_clock::now() - start_time).count();
    double avg_rate = total_received / total_elapsed;
    double drop_rate = (total_dropped * 100.0) / (total_received + total_dropped);

    std::cout << std::endl;
    std::cout << "===== FINAL REPORT =====" << std::endl;
    std::cout << "Total received: " << total_received << std::endl;
    std::cout << "Total dropped: " << total_dropped << " (" << drop_rate << "%)" << std::endl;
    std::cout << "Average rate: " << static_cast<int>(avg_rate) << " ticks/s" << std::endl;
    std::cout << "Trades: " << trade_count << " | Quotes: " << quote_count << std::endl;

    if (drop_rate < 1.0) {
        std::cout << "✅ SUCCESS: Drop rate < 1%" << std::endl;
    } else if (drop_rate < 5.0) {
        std::cout << "⚠️  WARNING: Drop rate between 1-5%" << std::endl;
    } else {
        std::cout << "❌ FAILURE: Drop rate > 5%" << std::endl;
    }

    return 0;
}
