#include "shared.hpp"
#include <iostream>
#include <cstring>

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
    std::cout << "TCP Receiver - Test 01" << std::endl;
    std::cout << "======================" << std::endl;

    // Parse arguments
    int port = 9001;  // Receive from SwitchSpy forward port

    if (argc > 1) port = std::atoi(argv[1]);

    std::cout << "Listening on port: " << port << std::endl;

#ifdef _WIN32
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        std::cerr << "WSAStartup failed" << std::endl;
        return 1;
    }
#endif

    // Create socket
    socket_t listen_sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (listen_sock == INVALID_SOCKET_FD) {
        std::cerr << "Failed to create socket" << std::endl;
        return 1;
    }

    // Bind
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

    // Listen
    if (listen(listen_sock, 1) < 0) {
        std::cerr << "Listen failed" << std::endl;
        close_socket(listen_sock);
        return 1;
    }

    std::cout << "Waiting for connection..." << std::endl;

    // Accept
    socket_t client_sock = accept(listen_sock, nullptr, nullptr);
    if (client_sock == INVALID_SOCKET_FD) {
        std::cerr << "Accept failed" << std::endl;
        close_socket(listen_sock);
        return 1;
    }

    std::cout << "Client connected!" << std::endl;
    std::cout << "Receiving messages..." << std::endl;

    // Receive messages
    uint32_t expected_seq = 0;
    uint64_t total_received = 0;
    uint64_t errors = 0;

    while (true) {
        SimpleMessage msg;
        ssize_t received = recv(client_sock, (char*)&msg, sizeof(msg), 0);

        if (received == 0) {
            std::cout << "Connection closed by sender" << std::endl;
            break;
        }

        if (received < 0) {
            std::cerr << "Receive failed" << std::endl;
            break;
        }

        if (received != sizeof(msg)) {
            std::cerr << "Partial message received: " << received << " bytes" << std::endl;
            errors++;
            continue;
        }

        total_received++;

        // Validate sequence
        if (msg.sequence_number != expected_seq) {
            std::cerr << "Sequence mismatch! Expected " << expected_seq
                      << ", got " << msg.sequence_number << std::endl;
            errors++;
            expected_seq = msg.sequence_number + 1;
        } else {
            expected_seq++;
        }

        std::cout << "Recv #" << msg.sequence_number
                  << " | Temp: " << msg.temperature << "°C"
                  << " | Status: " << (int)msg.status
                  << " | Total: " << total_received
                  << " | Errors: " << errors << std::endl;
    }

    close_socket(client_sock);
    close_socket(listen_sock);

#ifdef _WIN32
    WSACleanup();
#endif

    std::cout << std::endl;
    std::cout << "Summary:" << std::endl;
    std::cout << "  Total received: " << total_received << std::endl;
    std::cout << "  Errors: " << errors << std::endl;
    std::cout << "  Success rate: " << (total_received - errors) * 100.0 / total_received << "%" << std::endl;

    return 0;
}
