#pragma once

#include <winsock2.h>
#include <ws2tcpip.h>
#include <mstcpip.h>
#include <windows.h>
#include <clocale>
#include <string>

enum ContactStatus
{
    ACCEPT,
    SERVICED,
    FINISHED
};

enum FinishReason
{
    DONE,
    FAILED,
    TIMED_OUT,
    SERVER_EXIT
};

struct Contact
{
    SOCKET client_socket = INVALID_SOCKET;
    SOCKADDR_IN params = {};
    int lprms = sizeof(params);
    volatile LONG status = ACCEPT;
    HANDLE h_accept_thread = NULL;
    char service_name[50] = {};
    HANDLE h_timer = NULL;
    HANDLE h_service_thread = NULL;
    volatile LONG finish_reason = DONE;
    volatile LONG pending_apc = 0;
    bool timer_armed = false;
    int request_length = 0;
    ULONGLONG request_deadline = 0;
    SYSTEMTIME start_time = {};
    SYSTEMTIME finish_time = {};
    void (*log_message)(const std::string&) = nullptr;
};

inline void InitConsole()
{
    SetConsoleCP(CP_UTF8);
    SetConsoleOutputCP(CP_UTF8);
    setlocale(LC_ALL, ".UTF8");
}

inline LONG ReadCounter(volatile LONG& value)
{
    return InterlockedCompareExchange(&value, 0, 0);
}

inline bool SendAll(SOCKET socket, const char* data, int size)
{
    int sent = 0;
    while (sent < size)
    {
        int bytes = send(socket, data + sent, size - sent, 0);
        if (bytes <= 0)
        {
            return false;
        }
        sent += bytes;
    }
    return true;
}

inline bool SendLine(SOCKET socket, const std::string& text)
{
    std::string message = text + '\n';
    return SendAll(socket, message.data(), static_cast<int>(message.size()));
}

inline bool EnableKeepAlive(SOCKET socket)
{
    tcp_keepalive settings = { 1, 5000, 1000 };
    DWORD returned = 0;
    return WSAIoctl(socket, SIO_KEEPALIVE_VALS, &settings, sizeof(settings),
        NULL, 0, &returned, NULL, NULL) != SOCKET_ERROR;
}

inline int WaitForSocket(SOCKET socket, long milliseconds)
{
    fd_set read_set;
    FD_ZERO(&read_set);
    FD_SET(socket, &read_set);
    timeval timeout = { milliseconds / 1000, milliseconds % 1000 * 1000 };
    return select(0, &read_set, NULL, NULL, &timeout);
}

inline int ResolveServer(const std::string& host, int port, SOCKADDR_IN& address)
{
    addrinfo hints = {};
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_TCP;
    addrinfo* result = nullptr;
    int error = getaddrinfo(host.c_str(), std::to_string(port).c_str(), &hints, &result);
    if (error == 0)
    {
        address = *reinterpret_cast<const SOCKADDR_IN*>(result->ai_addr);
        freeaddrinfo(result);
    }
    return error;
}
