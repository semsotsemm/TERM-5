#define _WINSOCK_DEPRECATED_NO_WARNINGS
#include <iostream>
#include <winsock2.h>
#include <string>
#include <chrono>
#include <cstdint>
#include "../ServerCommon.h"

#pragma comment(lib, "ws2_32.lib")

using namespace std;




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


int GetNumber(const string& text, int minimum, int maximum)
{
    size_t end = 0;
    int value = stoi(text, &end);
    if (end != text.size() || value < minimum || value > maximum)
        throw string("Число должно быть в диапазоне " + to_string(minimum) + ".." + to_string(maximum) + ".\n");
    return value;
}

// Поиск по UDP без бесконечной блокировки, как требует задание 27.
string FindServer(int udp_port, const string& callsign, int& tcp_port)
{
    SOCKET udp_socket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (udp_socket == INVALID_SOCKET) throw SetErrorMessageText("Ошибка UDP socket: ", WSAGetLastError());
    BOOL broadcast = TRUE;
    u_long nonblk = 1;
    SOCKADDR_IN address = {};
    address.sin_family = AF_INET;
    address.sin_port = htons(static_cast<u_short>(udp_port));
    address.sin_addr.s_addr = INADDR_BROADCAST;
    if (setsockopt(udp_socket, SOL_SOCKET, SO_BROADCAST, (const char*)&broadcast, sizeof(broadcast)) == SOCKET_ERROR ||
        ioctlsocket(udp_socket, FIONBIO, &nonblk) == SOCKET_ERROR ||
        sendto(udp_socket, callsign.c_str(), static_cast<int>(callsign.size()), 0,
            (sockaddr*)&address, sizeof(address)) == SOCKET_ERROR)
    {
        int error = WSAGetLastError();
        closesocket(udp_socket);
        throw SetErrorMessageText("Ошибка отправки broadcast: ", error);
    }
    // Локальный экземпляр найдётся и при блокировке broadcast брандмауэром.
    address.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
    sendto(udp_socket, callsign.c_str(), static_cast<int>(callsign.size()), 0,
        (sockaddr*)&address, sizeof(address));
    cout << "Поиск сервера: UDP " << udp_port << ", позывной '" << callsign << "', ожидание до 3 секунд...\n";
    ULONGLONG deadline = GetTickCount64() + 3000;
    while (GetTickCount64() < deadline)
    {
        char buffer[256];
        SOCKADDR_IN server_address = {};
        int address_size = sizeof(server_address);
        int bytes = recvfrom(udp_socket, buffer, sizeof(buffer) - 1, 0, (sockaddr*)&server_address, &address_size);
        if (bytes > 0)
        {
            buffer[bytes] = '\0';
            string answer(buffer);
            if (answer.rfind("Welcome to ConcurrentServer; TCP=", 0) == 0)
            {
                closesocket(udp_socket);
                tcp_port = GetNumber(answer.substr(answer.find('=') + 1), 1, 65535);
                return inet_ntoa(server_address.sin_addr);
            }
        }
        else if (bytes == SOCKET_ERROR && WSAGetLastError() != WSAEWOULDBLOCK)
        {
            int error = WSAGetLastError();
            closesocket(udp_socket);
            throw SetErrorMessageText("Ошибка recvfrom: ", error);
        }
        Sleep(20);
    }
    closesocket(udp_socket);
    throw string("Сервер не найден за 3 секунды. Проверьте позывной и UDP-порт.\n");
}

int main(int argc, char* argv[])
{
    InitConsole();
    if (argc > 4 || (argc > 1 && string(argv[1]) == "--help"))
    {
        cout << "ClientT.exe [IPv4 сервера] [TCP-порт] [пауза между запросами, мс]\n"
             << "По умолчанию: 127.0.0.1 2000 0\n"
             << "ClientT.exe --discover [UDP-порт] [позывной] — поиск сервера в сети.\n"
             << "Для проверки минутного таймера: ClientT.exe 127.0.0.1 2000 1000, затем echo и 70 сообщений.\n";
        return argc > 4 ? 1 : 0;
    }
    WSADATA wsa_data;
    int error = WSAStartup(MAKEWORD(2, 2), &wsa_data);
    if (error != 0)
    {
        cerr << SetErrorMessageText("Ошибка WSAStartup: ", error);
        return 1;
    }
    SOCKET client_socket = INVALID_SOCKET;
    int exit_code = 0;
    try
    {
        string server_ip = "127.0.0.1";
        int server_port = 2000;
        int delay = 0;
        if (argc > 1 && string(argv[1]) == "--discover")
        {
            int udp_port = argc > 2 ? GetNumber(argv[2], 1, 65535) : 2000;
            string callsign = argc > 3 ? argv[3] : "FindServer";
            server_ip = FindServer(udp_port, callsign, server_port);
        }
        else
        {
            if (argc > 1) server_ip = argv[1];
            if (argc > 2) server_port = GetNumber(argv[2], 1, 65535);
            if (argc > 3) delay = GetNumber(argv[3], 0, 120000);
        }
        SOCKADDR_IN server_address = {};
        server_address.sin_family = AF_INET;
        server_address.sin_port = htons(static_cast<u_short>(server_port));
        server_address.sin_addr.s_addr = inet_addr(server_ip.c_str());
        if (server_address.sin_addr.s_addr == INADDR_NONE)
            throw string("Укажите корректный IPv4-адрес сервера.\n");
        client_socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
        if (client_socket == INVALID_SOCKET)
            throw SetErrorMessageText("Ошибка socket: ", WSAGetLastError());
        cout << "========== ClientT ==========\nПодключение к " << server_ip << ':' << server_port << "...\n";
        if (connect(client_socket, (sockaddr*)&server_address, sizeof(server_address)) == SOCKET_ERROR)
            throw SetErrorMessageText("Ошибка connect: ", WSAGetLastError());
        DWORD timeout = 65000;
        setsockopt(client_socket, SOL_SOCKET, SO_RCVTIMEO, (const char*)&timeout, sizeof(timeout));
        setsockopt(client_socket, SOL_SOCKET, SO_SNDTIMEO, (const char*)&timeout, sizeof(timeout));
        cout << "Соединение установлено.\n"
             << "  echo — возврат отправленных сообщений\n"
             << "  time — дата и время сервера\n"
             << "  rand — случайное 32-битное целое число\n"
             << "Другой код — проверка ErrorInquiry. Количество вводится отдельно.\n";
        string request_cmd;
        while (true)
        {
            cout << "\nСервис> " << flush;
            if (!getline(cin, request_cmd)) throw string("Ввод завершён.\n");
            if (!request_cmd.empty() && request_cmd.size() < 50 && request_cmd.find_first_of(" \t\r") == string::npos) break;
            cout << "Введите один код, например echo. Число сообщений будет запрошено отдельно.\n";
        }
        bool known_service = request_cmd == "echo" || request_cmd == "time" || request_cmd == "rand";
        int messages_count = 0;
        if (known_service)
        {
            while (true)
            {
                cout << "Количество сообщений (1..1000000)> " << flush;
                string value;
                if (!getline(cin, value)) throw string("Ввод завершён.\n");
                try
                {
                    messages_count = GetNumber(value, 1, 1000000);
                    break;
                }
                catch (...)
                {
                    cout << "Введите целое число от 1 до 1000000.\n";
                }
            }
        }
        if (!SendLine(client_socket, request_cmd))
            throw SetErrorMessageText("Ошибка отправки кода запроса: ", WSAGetLastError());
        string answer;
        if (ReceiveLine(client_socket, answer) != 1)
            throw string("Сервер отключился до подтверждения сервиса.\n");
        if (answer == "ErrorInquiry")
        {
            cout << "Сервер ответил ErrorInquiry: сервис '" << request_cmd << "' не существует.\n";
        }
        else if (answer != "OK")
        {
            throw string("Сервер не запустил сервис: " + answer + ".\n");
        }
        else
        {
            cout << "Сервис '" << request_cmd << "' запущен. Начинаем обмен.\n";
            auto start_time = chrono::steady_clock::now();
            for (int i = 1; i <= messages_count; i++)
            {
                string message = request_cmd == "echo" ? "Hello from Client " + to_string(i) : request_cmd;
                if (!SendLine(client_socket, message))
                    throw SetErrorMessageText("Ошибка отправки: ", WSAGetLastError());
                if (request_cmd == "rand")
                {
                    int32_t number;
                    if (!ReceiveAll(client_socket, reinterpret_cast<char*>(&number), sizeof(number)))
                        throw string("Сервер закрыл соединение при чтении rand. Возможен лимит 60 секунд.\n");
                    cout << '[' << i << '/' << messages_count << "] rand = " << number << '\n';
                }
                else
                {
                    if (ReceiveLine(client_socket, answer) != 1)
                        throw string("Сервер закрыл соединение при чтении ответа. Возможен лимит 60 секунд.\n");
                    cout << '[' << i << '/' << messages_count << "] " << answer << '\n';
                }
                if (delay > 0 && i < messages_count) Sleep(delay);
            }
            if (request_cmd != "echo" && !SendLine(client_socket, "stop"))
                throw string("Не удалось отправить завершение сервиса.\n");
            shutdown(client_socket, SD_SEND);
            char buffer[256];
            int received;
            do { received = recv(client_socket, buffer, sizeof(buffer), 0); } while (received > 0);
            if (received == SOCKET_ERROR)
                throw SetErrorMessageText("Ошибка завершения обмена: ", WSAGetLastError());
            double seconds = chrono::duration<double>(chrono::steady_clock::now() - start_time).count();
            cout << "Обмен завершён: " << messages_count << " ответов, " << seconds << " с.\n";
        }
    }
    catch (const string& message)
    {
        cerr << "[ClientT] " << message;
        exit_code = 1;
    }
    catch (const exception&)
    {
        cerr << "[ClientT] Некорректные параметры командной строки. Подсказка: ClientT.exe --help\n";
        exit_code = 1;
    }
    if (client_socket != INVALID_SOCKET) closesocket(client_socket);
    WSACleanup();
    cout << "Соединение закрыто.\n";
    return exit_code;
}
