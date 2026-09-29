#define _WINSOCK_DEPRECATED_NO_WARNINGS

#include <iostream>
#include <winsock2.h>
#include <string>
#include <ctime>

#pragma comment(lib, "ws2_32.lib")

using namespace std;

const char* SERVER_IP = "127.0.0.1";
const short SERVER_PORT = 2000;


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

bool GetRequestFromClient(char* name, short port, struct sockaddr* from, int* flen) 
{
    SOCKET server_socket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (server_socket == INVALID_SOCKET)
    {
        closesocket(server_socket);
        throw(SetErrorMessageText("Ошибка WinSosk: ", WSAGetLastError()));
    }

    SOCKADDR_IN server_address;
    server_address.sin_family = AF_INET;
    server_address.sin_port = htons(SERVER_PORT);
    server_address.sin_addr.s_addr =INADDR_ANY;

    if (bind(server_socket, (sockaddr*)&server_address, sizeof(server_address))== SOCKET_ERROR)
    {
        throw(SetErrorMessageText("Ошибка WinSosk: ", WSAGetLastError()));
    }

    int timeout = 10000;
    if (setsockopt(server_socket, SOL_SOCKET, SO_RCVTIMEO, (char*)&timeout, sizeof(timeout)) == SOCKET_ERROR)
    {
        closesocket(server_socket);
        throw(SetErrorMessageText("Ошибка WinSosk: ", WSAGetLastError()));
    }
    char receive_buffer[1024];
    cout << "Ожидание позывного '" << name << "' на порту " << port << "...\n";

    while (true) 
    {
        int bytes_received = recvfrom(server_socket, receive_buffer, sizeof(receive_buffer) - 1, 0, from, flen);
        if(bytes_received == SOCKET_ERROR)
        {
            closesocket(server_socket);
            int error_code = WSAGetLastError();
            if (error_code == WSAETIMEDOUT)
            {
                return false;
            }
            else
            {
                throw(SetErrorMessageText("Ошибка WinSosk: ", WSAGetLastError()));
            }
        }

        receive_buffer[bytes_received] = '\0';

        cout << "Пришло сообшение: " << receive_buffer << endl;
        if (strcmp(receive_buffer, name) == 0) 
        {
            cout << "Позывной верный.\n";
            closesocket(server_socket);
            return true;
        }
        else
        {
            cout << "Позывной не совпал. Игнорируем...\n";
        }
    }


    if (closesocket(server_socket) == SOCKET_ERROR)
    {
        throw(SetErrorMessageText("Ошибка WinSosk: ", WSAGetLastError()));
    }
}

bool PutAnswerToClient(char* name, struct sockaddr* to, int tlen) 
{
    SOCKET client_socket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);

    if (client_socket == INVALID_SOCKET)
    {
        throw(SetErrorMessageText("Ошибка WinSock: ", WSAGetLastError()));
    }

    int bytes_send = sendto(client_socket, name, strlen(name), 0, to, tlen);

    if (bytes_send == SOCKET_ERROR)
    {
        closesocket(client_socket);
        throw(SetErrorMessageText("Ошибка WinSock: ", WSAGetLastError()));
    }
    if (closesocket(client_socket) == SOCKET_ERROR)
    {
        throw(SetErrorMessageText("Ошибка WinSock: ", WSAGetLastError()));
    }

    return true;
}

// Поиска других серверов в локальной сети
void FindOtherServers(char* callsign, short port)
{
    cout << "Проверка наличия других серверов в сети...\n";

    SOCKET s = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (s == INVALID_SOCKET)
        throw SetErrorMessageText("Ошибка socket: ", WSAGetLastError());

    int bOptVal = 1;
    if (setsockopt(s, SOL_SOCKET, SO_BROADCAST, (char*)&bOptVal, sizeof(bOptVal)) == SOCKET_ERROR)
        throw SetErrorMessageText("Ошибка setsockopt (BROADCAST): ", WSAGetLastError());

    int timeout = 2000;
    if (setsockopt(s, SOL_SOCKET, SO_RCVTIMEO, (char*)&timeout, sizeof(timeout)) == SOCKET_ERROR)
        throw SetErrorMessageText("Ошибка setsockopt (TIMEOUT): ", WSAGetLastError());

    SOCKADDR_IN all;
    all.sin_family = AF_INET;
    all.sin_port = htons(port);
    all.sin_addr.s_addr = INADDR_BROADCAST;

    if (sendto(s, callsign, strlen(callsign), 0, (sockaddr*)&all, sizeof(all)) == SOCKET_ERROR)
        throw SetErrorMessageText("Ошибка sendto: ", WSAGetLastError());

    char receive_buffer[1024];
    SOCKADDR_IN from;
    int from_len = sizeof(from);
    int server_count = 0; 

    while (true)
    {
        int bytes_received = recvfrom(s, receive_buffer, sizeof(receive_buffer) - 1, 0, (sockaddr*)&from, &from_len);

        if (bytes_received == SOCKET_ERROR)
        {
            int err = WSAGetLastError();
            if (err == WSAETIMEDOUT)
            {
                break; 
            }
            else
            {
                closesocket(s);
                throw SetErrorMessageText("Ошибка recvfrom: ", err);
            }
        }

        receive_buffer[bytes_received] = '\0';
        if (strcmp(receive_buffer, callsign) == 0)
        {
            server_count++;
            cout << " -> Найден работающий сервер! IP-адрес: " << inet_ntoa(from.sin_addr) << endl;
        }
    }

    closesocket(s);

    if (server_count == 0)
    {
        cout << "Других серверов в сети не обнаружено.\n";
    }
    else
    {
        cout << "ВНИМАНИЕ! В локальной сети уже работают серверы с таким же позывным. Количество: " << server_count << "\n";
    }
    cout << "==============================================================\n";
}

int main()
{
    setlocale(LC_CTYPE, "Russian");
    WSADATA wsa_data;
    try 
    {
        if (WSAStartup(MAKEWORD(2, 2), &wsa_data) != 0) 
        {
            throw(SetErrorMessageText("Ошибка WinSosk: ", WSAGetLastError()));
        }
        char callsign[] = "Hello"; 
        FindOtherServers(callsign, 2000);
        cout << "Сервер запущен и ожидает запросов...\n";

        while (true)
        {
            SOCKADDR_IN client_address;

            int client_address_size = sizeof(client_address);

            bool result = GetRequestFromClient(callsign, SERVER_PORT, (sockaddr*)&client_address, &client_address_size);

            if (result)
            {
                cout << "Успех. Правильный позывной получен.\n";
                cout << "--- Параметры сокета подключившегося клиента ---\n";
                cout << "IP-адрес : " << inet_ntoa(client_address.sin_addr) << endl;
                cout << "Порт     : " << ntohs(client_address.sin_port) << endl;
                cout << "------------------------------------------------\n";

                if (PutAnswerToClient(callsign, (sockaddr*)&client_address, &client_address_size))
                {
                    cout << "Ответный позывной успешно отправлен клиенту.\n";
                }
            }
            else
            {
                cout << "Таймайт 10 сек.\n";
            }
        }

        WSACleanup();
    }
    catch (string error_message) 
    {
        return 1;
    }
    return 0;
}