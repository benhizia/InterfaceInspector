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

uint64_t get_timestamp_ms() {
    return std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::system_clock::now().time_since_epoch()
    ).count();
}

int main(int argc, char** argv) {
    std::cout << "TCP Sender - Test 01" << std::endl;
    std::cout << "====================" << std::endl;

    // Parse arguments
    const char* host = "127.0.0.1";
    int port = 8001;  // Connect to SwitchSpy relay port

    if (argc > 1) host = argv[1];
    if (argc > 2) port = std::atoi(argv[2]);

    std::cout << "Connecting to: " << host << ":" << port << std::endl;

#ifdef _WIN32
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        std::cerr << "WSAStartup failed" << std::endl;
        return 1;
    }
#endif

    // Create socket
    socket_t sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (sock == INVALID_SOCKET_FD) {
        std::cerr << "Failed to create socket" << std::endl;
        return 1;
    }

    // Connect
    struct sockaddr_in addr;
    std::memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    inet_pton(AF_INET, host, &addr.sin_addr);

    if (connect(sock, (struct sockaddr*)&addr, sizeof(addr)) < 0) {
        std::cerr << "Failed to connect" << std::endl;
        close_socket(sock);
        return 1;
    }

    std::cout << "Connected!" << std::endl;
    std::cout << "Sending messages (Ctrl+C to stop)..." << std::endl;

    // Send messages
    uint32_t sequence = 0;
    while (true) {
        SimpleMessage msg;
        msg.sequence_number = sequence++;
        msg.timestamp_ms = get_timestamp_ms();
        msg.temperature = 20.0f + (sequence % 100) * 0.5f;  // Simulate temperature
        msg.status = (sequence % 10 == 0) ? 1 : 0;  // Occasional warning
        msg.padding[0] = msg.padding[1] = msg.padding[2] = 0;

        ssize_t sent = send(sock, (char*)&msg, sizeof(msg), 0);
        if (sent < 0) {
            std::cerr << "Send failed" << std::endl;
            break;
        }

        std::cout << "Sent #" << msg.sequence_number
                  << " | Temp: " << msg.temperature << "°C"
                  << " | Status: " << (int)msg.status << std::endl;

        std::this_thread::sleep_for(std::chrono::milliseconds(500));
    }

    close_socket(sock);

#ifdef _WIN32
    WSACleanup();
#endif

    return 0;
}
