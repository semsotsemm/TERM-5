#define _WINSOCK_DEPRECATED_NO_WARNINGS

#include <iostream>
#include <winsock2.h>
#include <string>
#include <ctime>

#pragma comment(lib, "ws2_32.lib")

using namespace std;

const char* SERVER_IP = "127.0.0.1";
const int SERVER_PORT = 2000;


// Получение описания ошибки по ее коду.
string SetErrorMessageText(string message_text, int error_code) {
    string error_message = message_text;
    error_message += to_string(error_code) + ": ";
    switch (error_code) {
    case WSA_NOT_ENOUGH_MEMORY:
    {
        error_message += "Недостаточно оперативной памяти, для выполнения запроса.\n";
        break;
    }
    case WSAEACCES:
    {
        error_message += "Доступ запрещен. Недостаточно прав.\n";
        break;
    }
    case WSAEFAULT:
    {
        error_message += "В функию сетевого API был передан неверный указатель.\n";
        break;
    }
    case WSAEINVAL:
    {
        error_message += "В сетевую функцию передан неверный аргумент.\n";
        break;
    }
    case WSAEMFILE:
    {
        error_message += "Слишком много открытых файлов.\n";
        break;
    }
    case WSAEWOULDBLOCK:
    {
        error_message += "Ресурс временно недоступен.\n";
        break;
    }
    case WSAEINPROGRESS:
    {
        error_message += "Выполняется другая, блокирующая сетевая операция.\n";
        break;
    }
    case WSAEALREADY:
    {
        error_message += "Операция находится в процессе выполнения на данном сокете.\n";
        break;
    }
    case WSAENOTSOCK:
    {
        error_message += "Сокет задан неверно.\n";
        break;
    }
    case WSAEDESTADDRREQ:
    {
        error_message += "Требуется адрес назначения.\n";
        break;
    }
    case WSAEMSGSIZE:
    {
        error_message += "Сообщение слишком длинное\n";
        break;
    }
    case WSAEPROTOTYPE:
    {
        error_message += "Неправильный тип протокола для сокета.\n";
        break;
    }
    case WSAENOPROTOOPT:
    {
        error_message += "Передан неверный номер опции протокола.\n";
        break;
    }
    case WSAEPROTONOSUPPORT:
    {
        error_message += "Протокол не поддерживается на данной ОС.\n";
        break;
    }
    case WSAESOCKTNOSUPPORT:
    {
        error_message += "Тип сокета не поддерживается в данном семействе адресов.\n";
        break;
    }
    case WSAEOPNOTSUPP:
    {
        error_message += "Функия непреминима к данному типу сокета.\n";
        break;
    }
    case WSAEPFNOSUPPORT:
    {
        error_message += "Семейство протоколов не поддерживается на данном устройстве.\n";
        break;
    }
    case WSAEAFNOSUPPORT:
    {
        error_message += "Семейство адресов не поддерживается протоколом.\n";
        break;
    }
    case WSAEADDRINUSE:
    {
        error_message += "Адрес уже используется.\n";
        break;
    }
    case WSAENETDOWN:
    {
        error_message += "Сеть отключена.\n";
        break;
    }
    case WSAENETUNREACH:
    {
        error_message += "Сеть недосягаема.\n";
        break;
    }
    case WSAENETRESET:
    {
        error_message += "Сеть разорвала соединение.\n";
        break;
    }
    case WSAECONNABORTED:
    {
        error_message += "Соединение было разорвано принудительно со стороны ОС.\n";
        break;
    }
    case WSAECONNRESET:
    {
        error_message += "Клиент оборвал соединение.\n";
        break;
    }
    case WSAENOBUFS:
    {
        error_message += "Не хватает буфера, для отправки или приема данных.\n";
        break;
    }
    case WSAEISCONN:
    {
        error_message += "Сокет уже занят кем-то другим.\n";
        break;
    }
    case WSAENOTCONN:
    {
        error_message += "Сокет ни с кем не соединен.\n";
        break;
    }
    case WSAESHUTDOWN:
    {
        error_message += "Нельзя отправить: сокет был частично или полностью закрыт со стороны сервера.\n";
        break;
    }
    case WSAETIMEDOUT:
    {
        error_message += "Закончился интервал ожидания.\n";
        break;
    }
    case WSAECONNREFUSED:
    {
        error_message += "Соединение отклонено, никто не прослушивает заданный порт.\n";
        break;
    }
    case WSASYSNOTREADY:
    {
        error_message += "Сетевая подсистема Windows не готова к работе.\n";
        break;
    }
    case WSAVERNOTSUPPORTED:
    {
        error_message += "Указанная версия Winsock не найдена на данном устройстве.\n";
        break;
    }
    case WSANOTINITIALISED:
    {
        error_message += "Не выполнена инициализация WSAStartup.\n";
        break;
    }
    default:
    {
        error_message += "Неизвестная ошибка\n";
        break;
    }
    }
    return error_message;
}


int main()
{
    setlocale(LC_CTYPE, "Russian");
    WSADATA wsa_data;
    try
    {
        if (WSAStartup(MAKEWORD(2, 2), &wsa_data) != 0) 
        {
            throw(SetErrorMessageText("Ошибка WinSock: ", WSAGetLastError()));
        }

        SOCKET client_socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
        if (client_socket == INVALID_SOCKET) {
            throw SetErrorMessageText("Ошибка WinSock: ", WSAGetLastError());
        }
        cout << "Потоковый сокет успешно создан.\n";

        SOCKADDR_IN server_address;
        server_address.sin_family = AF_INET;           
        server_address.sin_port = htons(SERVER_PORT);
        server_address.sin_addr.s_addr = inet_addr(SERVER_IP);

        cout << "Попытка подключения к серверу...\n";

        if (connect(client_socket, (sockaddr*)&server_address, sizeof(server_address)) == SOCKET_ERROR)
        {
            throw SetErrorMessageText("Ошибка при подключении (connect): ", WSAGetLastError());
        }
        cout << "Успешное подключение к серверу!\n\n";

        int messages_count;
        cout << "Введите количество передаваемых сообщений: ";
        cin >> messages_count;

        cout << "\nНачинаем обмен данными. Пожалуйста, подождите...\n";

        clock_t start_time = clock();

        for (int i = 1; i <= messages_count; i++)
        {
            string message = "Hello from Client " + to_string(i);

            int bytes_sent = send(client_socket, message.c_str(), message.length(), 0);
            if (bytes_sent == SOCKET_ERROR)
                throw SetErrorMessageText("Ошибка send: ", WSAGetLastError());

            char buffer[1024];
            int bytes_received = recv(client_socket, buffer, sizeof(buffer) - 1, 0);
            if (bytes_received == SOCKET_ERROR)
                throw SetErrorMessageText("Ошибка recv: ", WSAGetLastError());

            buffer[bytes_received] = '\0';
        }

        clock_t end_time = clock();

        double total_time = (double)(end_time - start_time) / CLOCKS_PER_SEC;

        cout << "\nУспешно обменялись " << messages_count << " сообщениями!\n";
        cout << "Затраченное время: " << total_time << " секунд.\n";

        if (closesocket(client_socket) == SOCKET_ERROR) {
            throw SetErrorMessageText("Ошибка WinSock: ", WSAGetLastError());
        }
        cout << "Потоковый сокет успешно закрыт.\n";

        WSACleanup();
    }
    catch (string error_message) 
    {
        cerr << error_message;
        return 1;
    }
    return 0;
}