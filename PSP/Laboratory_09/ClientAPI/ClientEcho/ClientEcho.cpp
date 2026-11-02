#include "ClientAPI.h" 
#include <iostream>
#include <string>

using namespace std;

int main()
{
    setlocale(LC_ALL, "rus");
    ConcurrentServer client;

    cout << "Поиск сервера (UDP)..." << endl;
    if (!client.GetServer(2000, "FindServer"))
    {
        cout << "Сервер не найден." << endl;
        return 1;
    }

    cout << "Успех. Запрашиваем сервис 'echo'..." << endl;
    if (!client.Connect("echo"))
    {
        cout << "Сервер отклонил запрос." << endl;
        return 1;
    }

    cout << "Соединение установлено! Введите текст (или 'exit' для выхода)." << endl;
    cout << "[ТЕСТ ТАЙМАУТА]: Подождите 60 секунд ничего не вводя, чтобы проверить отключение сервером." << endl;

    string input;
    char buffer[1024];
    while (true)
    {
        getline(cin, input);
        if (input == "exit") break;

        memset(buffer, 0, sizeof(buffer));
        if (client.RequestEcho(input.c_str(), input.length(), buffer))
        {
            cout << "Ответ сервера: " << buffer << endl;
        }
        else
        {
            cout << "Сбой сети или сервер принудительно разорвал соединение (истек таймаут)." << endl;
            break;
        }
    }

    return 0;
}