#include "ClientAPI.h"
#include <iostream>

using namespace std;

int main()
{
    setlocale(LC_ALL, "rus");
    ConcurrentServer client;

    cout << "Поиск сервера..." << endl;
    if (!client.GetServer(2000, "FindServer")) return 1;

    cout << "Тест 1: Запрос несуществующего сервиса 'badtime'..." << endl;
    if (!client.Connect("badtime"))
    {
        cout << "Ожидаемый результат: Сервер отказал в доступе." << endl;
    }

    client.Disconnect();
    if (!client.GetServer(2000, "FindServer")) return 1;

    cout << "\nТест 2: Запрос корректного сервиса 'time'..." << endl;
    if (client.Connect("time"))
    {
        string time_str = client.RequestTime();
        if (!time_str.empty())
            cout << "Точное время на сервере: " << time_str << endl;
        else
            cout << "Ошибка получения данных." << endl;
    }
    else
    {
        cout << "Сбой подключения." << endl;
    }

    system("pause");
    return 0;
}