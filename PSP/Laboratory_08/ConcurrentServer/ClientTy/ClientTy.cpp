#define _WINSOCK_DEPRECATED_NO_WARNINGS
#include <iostream>
#include "../ServerCommon.h"
#pragma comment(lib, "ws2_32.lib")

using namespace std;

int main(int argc, char* argv[])
{
    InitConsole();
    WSADATA wsa_data;
    int error = WSAStartup(MAKEWORD(2, 2), &wsa_data);
    if (error != 0)
    {
        cerr << "Ошибка WSAStartup: " << error << '\n';
        return 1;
    }
    int exit_code = 0;
    try
    {
        if (argc > 4 || (argc > 1 && string(argv[1]) == "--help"))
        {
            cout << "ClientTy.exe [IPv4 сервера] [TCP-порт] [количество подключений]\n"
                 << "По умолчанию: 127.0.0.1 2000 100\n";
            WSACleanup();
            return argc > 4 ? 1 : 0;
        }
        string server_ip = argc > 1 ? argv[1] : "127.0.0.1";
        auto get_number = [](const char* text, int maximum)
        {
            size_t end;
            int number = stoi(text, &end);
            if (text[end] != '\0' || number < 1 || number > maximum) throw invalid_argument("number");
            return number;
        };
        int port = argc > 2 ? get_number(argv[2], 65535) : 2000;
        int connections_count = argc > 3 ? get_number(argv[3], 1000000) : 100;
        SOCKADDR_IN server_address = {};
        server_address.sin_family = AF_INET;
        server_address.sin_port = htons(static_cast<u_short>(port));
        server_address.sin_addr.s_addr = inet_addr(server_ip.c_str());
        if (server_address.sin_addr.s_addr == INADDR_NONE) throw invalid_argument("IPv4");
        cout << "========== ClientTy: тест подключений ==========\nСервер: " << server_ip << ':' << port
             << "\nПодключений: " << connections_count << ". После подключения сразу закрываем соединение.\n";
        int successful = 0;
        for (int i = 1; i <= connections_count; i++)
        {
            SOCKET client_socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
            if (client_socket != INVALID_SOCKET &&
                connect(client_socket, (sockaddr*)&server_address, sizeof(server_address)) != SOCKET_ERROR)
            {
                successful++;
                shutdown(client_socket, SD_SEND);
                cout << '[' << i << '/' << connections_count << "] Подключение успешно, отключаемся.\n";
            }
            else
            {
                cout << '[' << i << '/' << connections_count << "] Ошибка подключения: " << WSAGetLastError() << '\n';
            }
            if (client_socket != INVALID_SOCKET) closesocket(client_socket);
            Sleep(20);
        }
        cout << "Тест завершён. Подключений: " << successful << ", ошибок: " << connections_count - successful
             << ".\nНа сервере эти отключения до выбора сервиса учитываются как отказы.\n";
        exit_code = successful == connections_count ? 0 : 1;
    }
    catch (const exception&)
    {
        cerr << "Некорректный адрес или числовой параметр. Подсказка: ClientTy.exe --help\n";
        exit_code = 1;
    }
    WSACleanup();
    return exit_code;
}
