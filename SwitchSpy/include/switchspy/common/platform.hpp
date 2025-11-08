#pragma once

// Platform detection
#if defined(_WIN32) || defined(_WIN64)
    #define SWITCHSPY_PLATFORM_WINDOWS
    #include <winsock2.h>
    #include <ws2tcpip.h>
    #pragma comment(lib, "ws2_32.lib")

    using socket_t = SOCKET;
    #define INVALID_SOCKET_FD INVALID_SOCKET
    #define close_socket closesocket

#elif defined(__linux__)
    #define SWITCHSPY_PLATFORM_LINUX
    #include <sys/socket.h>
    #include <sys/epoll.h>
    #include <netinet/in.h>
    #include <arpa/inet.h>
    #include <unistd.h>
    #include <fcntl.h>
    #include <errno.h>

    using socket_t = int;
    #define INVALID_SOCKET_FD -1
    #define close_socket close

#elif defined(__APPLE__)
    #define SWITCHSPY_PLATFORM_MACOS
    #include <sys/socket.h>
    #include <sys/event.h>
    #include <netinet/in.h>
    #include <arpa/inet.h>
    #include <unistd.h>
    #include <fcntl.h>
    #include <errno.h>

    using socket_t = int;
    #define INVALID_SOCKET_FD -1
    #define close_socket close

#else
    #error "Unsupported platform"
#endif

namespace switchspy {
namespace platform {

// Platform-specific initialization
inline bool initialize_networking() {
#ifdef SWITCHSPY_PLATFORM_WINDOWS
    WSADATA wsaData;
    return WSAStartup(MAKEWORD(2, 2), &wsaData) == 0;
#else
    return true; // Unix platforms don't require initialization
#endif
}

// Platform-specific cleanup
inline void cleanup_networking() {
#ifdef SWITCHSPY_PLATFORM_WINDOWS
    WSACleanup();
#endif
}

// Set socket to non-blocking mode
inline bool set_non_blocking(socket_t sock) {
#ifdef SWITCHSPY_PLATFORM_WINDOWS
    u_long mode = 1;
    return ioctlsocket(sock, FIONBIO, &mode) == 0;
#else
    int flags = fcntl(sock, F_GETFL, 0);
    if (flags == -1) return false;
    return fcntl(sock, F_SETFL, flags | O_NONBLOCK) != -1;
#endif
}

// Get last socket error
inline int get_last_error() {
#ifdef SWITCHSPY_PLATFORM_WINDOWS
    return WSAGetLastError();
#else
    return errno;
#endif
}

// Check if error is "would block"
inline bool is_would_block(int error) {
#ifdef SWITCHSPY_PLATFORM_WINDOWS
    return error == WSAEWOULDBLOCK;
#else
    return error == EWOULDBLOCK || error == EAGAIN;
#endif
}

} // namespace platform
} // namespace switchspy
