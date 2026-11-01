#pragma once
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <winsock2.h>
#include <windows.h>
#include <string>
#include <clocale>

// Общая структура подключения для сервера и ServiceLibrary.dll.
enum ContactStatus
{
    ACCEPT,
    SERVICED,
    FINISHED
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
    volatile LONG is_error = FALSE;
    volatile LONG pending_apc = 0;
    bool timer_armed = false;
    int request_length = 0;
    ULONGLONG request_deadline = 0;
    SYSTEMTIME start_time = {};
    SYSTEMTIME finish_time = {};
    void (*log_message)(const std::string&) = nullptr;
};

inline LONG ReadCounter(volatile LONG& value)
{
    return InterlockedCompareExchange(&value, 0, 0);
}

inline void InitConsole()
{
    SetConsoleCP(CP_UTF8);
    SetConsoleOutputCP(CP_UTF8);
    setlocale(LC_ALL, ".UTF8");
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

// TCP не сохраняет границы send: строки отделяются переводом строки.
// 1 — строка получена, 0 — EOF, -1 — ошибка/слишком длинная строка.
inline int ReceiveLine(SOCKET socket, std::string& text)
{
    text.clear();
    char symbol;
    while (true)
    {
        int bytes = recv(socket, &symbol, 1, 0);
        if (bytes <= 0)
        {
            return bytes == 0 && text.empty() ? 0 : -1;
        }
        if (symbol == '\n')
        {
            return 1;
        }
        text += symbol;
        if (text.size() > 4096)
        {
            return -1;
        }
    }
}

inline bool ReceiveAll(SOCKET socket, char* data, int size)
{
    int received = 0;
    while (received < size)
    {
        int bytes = recv(socket, data + received, size - received, 0);
        if (bytes <= 0)
        {
            return false;
        }
        received += bytes;
    }
    return true;
}
