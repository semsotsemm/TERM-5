#include "ConsoleAPI.h"
#include <iostream>

using namespace std;

RemoteConsole::RemoteConsole() : h_pipe(INVALID_HANDLE_VALUE) {}

RemoteConsole::~RemoteConsole()
{
    Disconnect();
}

bool RemoteConsole::Connect(const string& server_name)
{
    pipe_name = "\\\\" + server_name + "\\pipe\\ServerPipe";

    while (true)
    {
        // Пытаемся открыть существующий именованный канал
        h_pipe = CreateFileA(
            pipe_name.c_str(),
            GENERIC_READ | GENERIC_WRITE,
            0,
            NULL,
            OPEN_EXISTING,
            0,
            NULL
        );

        if (h_pipe != INVALID_HANDLE_VALUE)
        {
            break; // Успешно подключились
        }

        // Если ошибка не связана с тем, что канал занят - выходим
        if (GetLastError() != ERROR_PIPE_BUSY)
        {
            return false;
        }

        // Если канал занят другим процессом, ждем 5 секунд
        if (!WaitNamedPipeA(pipe_name.c_str(), 5000))
        {
            return false;
        }
    }

    // Переводим канал в режим чтения сообщений (а не сплошного потока байт)
    DWORD mode = PIPE_READMODE_MESSAGE;
    if (!SetNamedPipeHandleState(h_pipe, &mode, NULL, NULL))
    {
        Disconnect();
        return false;
    }

    return true;
}

bool RemoteConsole::SendCommand(const string& command, string& response)
{
    if (h_pipe == INVALID_HANDLE_VALUE) return false;

    DWORD bytes_written = 0;
    if (!WriteFile(h_pipe, command.c_str(), command.length(), &bytes_written, NULL))
    {
        return false;
    }

    char buffer[4096] = {};
    DWORD bytes_read = 0;
    if (!ReadFile(h_pipe, buffer, sizeof(buffer) - 1, &bytes_read, NULL))
    {
        return false;
    }

    response = string(buffer, bytes_read);
    return true;
}

void RemoteConsole::Disconnect()
{
    if (h_pipe != INVALID_HANDLE_VALUE)
    {
        CloseHandle(h_pipe);
        h_pipe = INVALID_HANDLE_VALUE;
    }
}