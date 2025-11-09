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

int main(int argc, char** argv) {
    std::cout << "===== HIGH THROUGHPUT SENDER - Market Data Feed =====" << std::endl;

    // Parse arguments
    const char* host = "127.0.0.1";
    int port = 8004;
    int delay_us = 100;  // 100 microseconds = 10,000 ticks/second

    if (argc > 1) host = argv[1];
    if (argc > 2) port = std::atoi(argv[2]);
    if (argc > 3) delay_us = std::atoi(argv[3]);

    double rate_per_sec = 1000000.0 / delay_us;
    double throughput_mbps = (rate_per_sec * sizeof(MarketTick)) / (1024.0 * 1024.0);

    std::cout << "Configuration:" << std::endl;
    std::cout << "  Target: " << host << ":" << port << std::endl;
    std::cout << "  Tick rate: " << static_cast<int>(rate_per_sec) << " ticks/second" << std::endl;
    std::cout << "  Throughput: " << throughput_mbps << " MB/s" << std::endl;
    std::cout << "  Delay: " << delay_us << " microseconds" << std::endl;
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

    // Disable Nagle's algorithm for low latency
    int flag = 1;
    setsockopt(sock, IPPROTO_TCP, TCP_NODELAY, (char*)&flag, sizeof(flag));

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

    std::cout << "Connected! Starting high-frequency transmission..." << std::endl;
    std::cout << std::endl;

    // Random number generation for market data
    std::random_device rd;
    std::mt19937_64 gen(rd());
    std::uniform_int_distribution<uint64_t> symbol_dist(1, 10);  // 10 different symbols
    std::uniform_int_distribution<uint64_t> price_dist(1000000, 5000000);  // $100-$500
    std::uniform_int_distribution<uint32_t> size_dist(100, 10000);  // 100-10k shares
    std::uniform_int_distribution<uint8_t> phase_dist(2, 3);  // Open or Close

    uint64_t sequence = 0;
    uint64_t total_sent = 0;
    auto start_time = std::chrono::steady_clock::now();
    auto last_report = start_time;

    while (true) {
        MarketTick tick{};
        tick.symbol_id = symbol_dist(gen);
        tick.sequence = ++sequence;

        auto now = std::chrono::high_resolution_clock::now();
        tick.timestamp_ns = std::chrono::duration_cast<std::chrono::nanoseconds>(
            now.time_since_epoch()).count();

        // Generate bid/ask spread
        uint64_t mid_price = price_dist(gen);
        uint64_t spread = mid_price / 1000;  // 0.1% spread
        tick.bid_price_e4 = mid_price - spread / 2;
        tick.ask_price_e4 = mid_price + spread / 2;
        tick.bid_size = size_dist(gen);
        tick.ask_size = size_dist(gen);

        // Last trade (occasionally)
        if (sequence % 10 == 0) {
            tick.last_price_e4 = mid_price;
            tick.last_size = size_dist(gen) / 10;
            tick.flags = 0x01;  // is_trade
        } else {
            tick.last_price_e4 = 0;
            tick.last_size = 0;
            tick.flags = 0x02;  // is_quote
        }

        tick.market_phase = phase_dist(gen);
        tick.padding = 0;

        // Send tick
        ssize_t sent = send(sock, (char*)&tick, sizeof(tick), 0);
        if (sent != sizeof(tick)) {
            std::cerr << "Send failed or incomplete" << std::endl;
            break;
        }

        total_sent++;

        // Report every 1000 ticks
        if (sequence % 1000 == 0) {
            auto now_report = std::chrono::steady_clock::now();
            double elapsed = std::chrono::duration<double>(now_report - last_report).count();
            double rate = 1000.0 / elapsed;
            double total_elapsed = std::chrono::duration<double>(now_report - start_time).count();
            double avg_rate = total_sent / total_elapsed;
            double throughput = (avg_rate * sizeof(MarketTick)) / (1024.0 * 1024.0);

            std::cout << "Seq: " << sequence
                      << " | Sent: " << total_sent
                      << " | Rate: " << static_cast<int>(rate) << " ticks/s"
                      << " | Avg: " << static_cast<int>(avg_rate) << " ticks/s"
                      << " | Throughput: " << throughput << " MB/s"
                      << std::endl;

            last_report = now_report;
        }

        // Microsecond delay
        if (delay_us > 0) {
            std::this_thread::sleep_for(std::chrono::microseconds(delay_us));
        }
    }

    closesocket(sock);
#ifdef _WIN32
    WSACleanup();
#endif

    return 0;
}
