#include "shared.hpp"
#include <iostream>
#include <cstring>
#include <chrono>
#include <thread>
#include <random>

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
    std::cout << "===== FAST SENDER - Rate Mismatch Test =====" << std::endl;
    std::cout << "This sender transmits at HIGH SPEED to stress" << std::endl;
    std::cout << "test the buffer and flow control mechanisms." << std::endl;
    std::cout << "=============================================" << std::endl;
    std::cout << std::endl;

    const char* host = "127.0.0.1";
    int port = 8005;
    int send_delay_ms = 10;  // FAST: 100 packets/second

    if (argc > 1) host = argv[1];
    if (argc > 2) port = std::atoi(argv[2]);
    if (argc > 3) send_delay_ms = std::atoi(argv[3]);

    std::cout << "Configuration:" << std::endl;
    std::cout << "  Target: " << host << ":" << port << std::endl;
    std::cout << "  Send rate: " << 1000/send_delay_ms << " packets/second" << std::endl;
    std::cout << "  Delay between sends: " << send_delay_ms << "ms" << std::endl;
    std::cout << std::endl;

#ifdef _WIN32
    WSADATA wsaData;
    WSAStartup(MAKEWORD(2, 2), &wsaData);
#endif

    socket_t sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (sock == INVALID_SOCKET_FD) {
        std::cerr << "Failed to create socket" << std::endl;
        return 1;
    }

    struct sockaddr_in addr;
    std::memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    inet_pton(AF_INET, host, &addr.sin_addr);

    std::cout << "Connecting..." << std::endl;
    if (connect(sock, (struct sockaddr*)&addr, sizeof(addr)) < 0) {
        std::cerr << "Failed to connect" << std::endl;
        close_socket(sock);
        return 1;
    }

    std::cout << "Connected! Starting fast transmission..." << std::endl;
    std::cout << std::endl;

    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> temp_dist(1500, 3500);   // 15-35°C
    std::uniform_int_distribution<> humidity_dist(300, 900); // 30-90%
    std::uniform_int_distribution<> pressure_dist(98000, 102000); // Pa

    uint64_t sequence = 0;
    uint64_t total_sent = 0;
    auto start_time = std::chrono::steady_clock::now();

    while (true) {
        SensorData data;
        data.sequence = sequence++;
        data.sensor_id = 12345;
        data.temperature_c100 = temp_dist(gen);
        data.humidity_p10 = humidity_dist(gen);
        data.pressure_pa = pressure_dist(gen);
        data.status_flags = (sequence % 100 == 0) ? 0x01 : 0x00;  // Occasional alert
        data.battery_percent = 100 - (sequence % 100);
        data.reserved = 0;

        ssize_t sent = send(sock, (char*)&data, sizeof(data), 0);
        if (sent < 0) {
            std::cerr << "Send failed at sequence " << sequence << std::endl;
            break;
        }

        total_sent++;

        // Print status every 100 packets
        if (sequence % 100 == 0) {
            auto now = std::chrono::steady_clock::now();
            auto elapsed = std::chrono::duration_cast<std::chrono::seconds>(now - start_time).count();
            double rate = total_sent / std::max(1.0, (double)elapsed);

            std::cout << "Seq: " << sequence
                      << " | Sent: " << total_sent
                      << " | Rate: " << (int)rate << " pkt/s"
                      << " | Temp: " << data.temperature_c100/100.0 << "°C"
                      << " | Batt: " << (int)data.battery_percent << "%"
                      << std::endl;
        }

        std::this_thread::sleep_for(std::chrono::milliseconds(send_delay_ms));
    }

    close_socket(sock);

#ifdef _WIN32
    WSACleanup();
#endif

    return 0;
}
