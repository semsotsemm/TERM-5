#include "ConsoleAPI.h"
#include <iostream>
#include <string>

using namespace std;

int main()
{
    RemoteConsole console;

    cout << "Подключение к серверу управления (Именованный канал)..." << endl;

    if (!console.Connect("."))
    {
        cout << "Ошибка: не удалось подключиться к консоли сервера. Сервер запущен?" << endl;
        system("pause");
        return 1;
    }

    cout << "Подключение успешно установлено!" << endl;
    cout << "Доступные команды: start, stop, wait, shutdown, statistics, exit" << endl;
    cout << "-----------------------------------------------------------------" << endl;

    string command;
    string response;

    while (true)
    {
        cout << "RConsole> ";
        getline(cin, command);

        if (command.empty()) continue;

        if (!console.SendCommand(command, response))
        {
            cout << "Ошибка передачи данных. Связь с сервером разорвана." << endl;
            break;
        }

        cout << "Ответ сервера:\n" << response << "\n" << endl;

        if (command == "exit" || command == "shutdown")
        {
            cout << "Завершение работы консоли." << endl;
            break;
        }
    }

    console.Disconnect();
    system("pause");
    return 0;
}