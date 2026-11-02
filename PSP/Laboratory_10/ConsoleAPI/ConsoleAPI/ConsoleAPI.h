#pragma once
#include <string>
#include <windows.h>

class RemoteConsole
{
private:
    HANDLE h_pipe;
    std::string pipe_name;

public:
    RemoteConsole();
    ~RemoteConsole();

    // Подключение к каналу сервера (по умолчанию локально ".")
    bool Connect(const std::string& server_name = ".");

    // Отправка команды и получение ответа
    bool SendCommand(const std::string& command, std::string& response);

    // Отключение
    void Disconnect();
};