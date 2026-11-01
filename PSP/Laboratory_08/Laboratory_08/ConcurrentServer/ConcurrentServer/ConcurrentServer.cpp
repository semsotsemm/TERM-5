#define _WINSOCK_DEPRECATED_NO_WARNINGS

#pragma comment(lib, "WS2_32.lib")

#include <iostream>
#include <string>
#include <winsock2.h>
#include <windows.h>

using namespace std;

int PORT = 2000;

enum TalkersCommand {
    START,
    STOP,
    EXIT,
    STATISTICS,
    WAIT,
    SHUTDOWN,
    GETCOMMAND
};

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


void CommandsCycle(TalkersCommand& cmd)
{
    int squirt = 0;

    while (cmd != EXIT)
    {
        switch (cmd)
        {
        case START:
        {
            cout << "[CommandsCycle] Команда START: возобновление подключения клиентов.\n";
            cmd = GETCOMMAND;
            squirt = 10;
            break;
        }
        case STOP:
        {
            cout << "[CommandsCycle] Команда STOP: остановка подключения клиентов.\n";
            cmd = GETCOMMAND;
            squirt = 0;
            break;
        }
        case GETCOMMAND:
        {
            break;
        }
        }
        Sleep(1000);
    }

    cout << "[CommandsCycle] Команда EXIT: завершение цикла обработки команд.\n";
}


DWORD WINAPI AcceptServer(LPVOID lp_param) 
{
    cout << "[AcceptServer] Поток успешно запущен.\n";

    TalkersCommand* command = (TalkersCommand*)lp_param;
    cout << "[AcceptServer] Получена команда управления: " << *command << '\n';
    
    SOCKET server_socket = socket(AF_INET, SOCK_STREAM, NULL);
    if (server_socket == INVALID_SOCKET)
    {
        cout << SetErrorMessageText("Ошибка WinSock (socket): ", WSAGetLastError());
        return -1;
    }
    cout << "[AcceptServer] Потоковый сокет успешно создан.\n";

    SOCKADDR_IN server_address;
    server_address.sin_family = AF_INET;
    server_address.sin_port = htons(PORT);
    server_address.sin_addr.s_addr = INADDR_ANY;

    if (bind(server_socket, (sockaddr*)&server_address, sizeof(server_address)) == SOCKET_ERROR)
    {
        cout << SetErrorMessageText("Ошибка WinSock (bind): ", WSAGetLastError());
        closesocket(server_socket);
        return -1;
    }

    if (listen(server_socket, SOMAXCONN) == SOCKET_ERROR) 
    {
        cout << SetErrorMessageText("Ошибка WinSock (listen): ", WSAGetLastError());
        closesocket(server_socket);
        return -1;
    }
    cout << "[AcceptServer] Сервер запущен и готов к работе.\n";

    u_long nonblk = 1;
    if (ioctlsocket(server_socket, FIONBIO, &nonblk) == SOCKET_ERROR)
    {
        cout << SetErrorMessageText("Ошибка ioctlsocket: ", WSAGetLastError());
        closesocket(server_socket);
        return -1;
    }
    cout << "[AcceptServer] Сокет переведен в неблокирующий режим.\n";

    CommandsCycle(*command);

    closesocket(server_socket);
    cout << "[AcceptServer] Поток завершает работу.\n";

    return 0;
}


int main(int argc, char* argv[])
{
    setlocale(LC_CTYPE, "Russian");
    
    WSADATA wsa_data;
    
    try 
    {
        if (WSAStartup(MAKEWORD(2, 2), &wsa_data) != 0) 
        {
            throw SetErrorMessageText("Ошибка WinSock: ", WSAGetLastError());
        }
        cout << "Инициализация библиотеки прошла успешно. ConcurrentServer успешно запущен.\n";

        HANDLE h_accept_server;
        DWORD dw_accept_server_id;

        if (argc > 1)
        {
            PORT = atoi(argv[1]);
        }

        TalkersCommand cmd_start = START;
        
        // Атрибуты безопастности, размер стека по умолчанию, указатель на функцию, параметры функции, флаги (немедленный запуск), структура для id нового потока.
        h_accept_server = CreateThread(NULL, 0, AcceptServer, &cmd_start, 0, &dw_accept_server_id);
        if (h_accept_server == NULL)
        {
            cerr << "Ошибка при создании потока AcceptServer.";
            WSACleanup();
            return -1;
        }

        WaitForSingleObject(h_accept_server, INFINITE);
        CloseHandle(h_accept_server);

        WSACleanup();
    }
    catch (string error_message) 
    {
        cerr << error_message;
        return 1;
    }

    system("pause");

    return 0;
}