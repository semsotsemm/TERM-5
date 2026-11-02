#include "ClientAPI.h"
#include <iostream>

using namespace std;

int main()
{
    setlocale(LC_ALL, "rus");
    ConcurrentServer client;

    cout << "Поиск сервера..." << endl;
    if (!client.GetServer(2000, "FindServer"))
    {
        cout << "Сервер не найден." << endl;
        return 1;
    }

    cout << "Запрашиваем сервис 'rand'..." << endl;
    if (client.Connect("rand"))
    {
        int32_t random_number = client.RequestRand();
        if (random_number != 0)
        {
            cout << "Сервер прислал случайное число: " << random_number << endl;
        }
        else
        {
            cout << "Ошибка получения числа или получено число 0." << endl;
        }
    }
    else
    {
        cout << "Сбой подключения к сервису." << endl;
    }

    system("pause");
    return 0;
}