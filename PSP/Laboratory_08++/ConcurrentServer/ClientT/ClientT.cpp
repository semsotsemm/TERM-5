#define _WINSOCK_DEPRECATED_NO_WARNINGS
#include <iostream>
#include "../ServerCommon.h"
#include <mswsock.h>
#include <vector>
#include <algorithm>
#include <string>
#include <chrono>

#pragma comment(lib, "ws2_32.lib")

using namespace std;


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

string FindServer(int udp_port, const string& callsign, int& tcp_port)
{
    SOCKET udp_socket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (udp_socket == INVALID_SOCKET) throw SetErrorMessageText("Ошибка UDP socket: ", WSAGetLastError());
    try
    {
        BOOL broadcast = TRUE;
        BOOL report_port_unreachable = FALSE;
        DWORD returned = 0;
        u_long nonblk = 1;
        if (setsockopt(udp_socket, SOL_SOCKET, SO_BROADCAST, (const char*)&broadcast, sizeof(broadcast)) == SOCKET_ERROR ||
            WSAIoctl(udp_socket, SIO_UDP_CONNRESET, &report_port_unreachable,
                sizeof(report_port_unreachable), NULL, 0, &returned, NULL, NULL) == SOCKET_ERROR ||
            ioctlsocket(udp_socket, FIONBIO, &nonblk) == SOCKET_ERROR)
        {
            throw SetErrorMessageText("Ошибка настройки UDP-поиска: ", WSAGetLastError());
        }

        vector<u_long> destinations = { INADDR_BROADCAST, htonl(INADDR_LOOPBACK) };
        INTERFACE_INFO interfaces[64];
        if (WSAIoctl(udp_socket, SIO_GET_INTERFACE_LIST, NULL, 0, interfaces,
            sizeof(interfaces), &returned, NULL, NULL) != SOCKET_ERROR)
        {
            for (DWORD i = 0; i < returned / sizeof(INTERFACE_INFO); i++)
            {
                if ((interfaces[i].iiFlags & (IFF_UP | IFF_BROADCAST)) != (IFF_UP | IFF_BROADCAST)) continue;
                u_long address = interfaces[i].iiAddress.AddressIn.sin_addr.s_addr;
                u_long mask = interfaces[i].iiNetmask.AddressIn.sin_addr.s_addr;
                if (mask == 0 || mask == INADDR_BROADCAST) continue;
                u_long subnet_broadcast = (address & mask) | ~mask;
                if (find(destinations.begin(), destinations.end(), subnet_broadcast) == destinations.end())
                    destinations.push_back(subnet_broadcast);
            }
        }

        cout << "Поиск сервера: UDP " << udp_port << ", позывной '" << callsign << "', ожидание до 3 секунд...\n" << flush;
        ULONGLONG deadline = GetTickCount64() + 3000;
        ULONGLONG next_request = 0;
        while (GetTickCount64() < deadline)
        {
            if (GetTickCount64() >= next_request)
            {
                bool sent = false;
                int send_error = 0;
                SOCKADDR_IN address = {};
                address.sin_family = AF_INET;
                address.sin_port = htons(static_cast<u_short>(udp_port));
                for (u_long destination : destinations)
                {
                    address.sin_addr.s_addr = destination;
                    if (sendto(udp_socket, callsign.c_str(), static_cast<int>(callsign.size()), 0,
                        (sockaddr*)&address, sizeof(address)) != SOCKET_ERROR)
                        sent = true;
                    else send_error = WSAGetLastError();
                }
                if (!sent) throw SetErrorMessageText("Ошибка отправки запросов поиска: ", send_error);
                next_request = GetTickCount64() + 750;
            }

            char buffer[256];
            SOCKADDR_IN server_address = {};
            int address_size = sizeof(server_address);
            int bytes = recvfrom(udp_socket, buffer, sizeof(buffer) - 1, 0, (sockaddr*)&server_address, &address_size);
            if (bytes > 0 && ntohs(server_address.sin_port) == udp_port)
            {
                string answer(buffer, bytes);
                const string prefix = "Welcome to ConcurrentServer; TCP=";
                if (answer.rfind(prefix, 0) == 0)
                {
                    int found_port;
                    try { found_port = GetNumber(answer.substr(prefix.size()), 1, 65535); }
                    catch (...) { continue; }
                    string server_ip = inet_ntoa(server_address.sin_addr);
                    tcp_port = found_port;
                    closesocket(udp_socket);
                    return server_ip;
                }
            }
            else if (bytes == SOCKET_ERROR)
            {
                int error = WSAGetLastError();
                if (error != WSAEWOULDBLOCK && error != WSAEMSGSIZE)
                    throw SetErrorMessageText("Ошибка recvfrom: ", error);
            }
            Sleep(20);
        }
        throw string("Сервер не найден за 3 секунды. Проверьте позывной и UDP-порт.\n");
    }
    catch (...)
    {
        closesocket(udp_socket);
        throw;
    }
}


class ConsoleStatus
{
    HANDLE output = GetStdHandle(STD_OUTPUT_HANDLE);
    bool enabled = false;
    string previous;
    ULONGLONG last_draw = 0;
    COORD position = {};

    SHORT DrawLine(const string& text, SHORT width, SHORT row)
    {
        int length = MultiByteToWideChar(CP_UTF8, 0, text.data(), static_cast<int>(text.size()), NULL, 0);
        wstring wide(length, L' ');
        if (length > 0)
        {
            MultiByteToWideChar(CP_UTF8, 0, text.data(), static_cast<int>(text.size()), &wide[0], length);
        }
        wide.resize(width, L' ');
        DWORD written;
        WriteConsoleOutputCharacterW(output, wide.data(), static_cast<DWORD>(width), { 0, row }, &written);
        return static_cast<SHORT>(length < width ? length : width - 1);
    }

public:
    ConsoleStatus()
    {
        CONSOLE_SCREEN_BUFFER_INFO info;
        enabled = GetConsoleScreenBufferInfo(output, &info) != FALSE;
    }

    ~ConsoleStatus()
    {
        Clear();
    }

    void Clear()
    {
        if (!previous.empty())
        {
            CONSOLE_SCREEN_BUFFER_INFO info;
            if (GetConsoleScreenBufferInfo(output, &info) && position.Y + 1 < info.dwSize.Y)
            {
                DrawLine("", info.dwSize.X, position.Y);
                DrawLine("", info.dwSize.X, position.Y + 1);
                SetConsoleCursorPosition(output, position);
            }
            previous.clear();
        }
    }

    bool IsEnabled() const
    {
        return enabled;
    }

    void Show(ULONGLONG deadline, const string& line, bool limit_redraw = false)
    {
        if (!enabled)
        {
            return;
        }
        ULONGLONG now = GetTickCount64();
        if (limit_redraw && !previous.empty() && now - last_draw < 100)
        {
            return;
        }
        string status = "Ожидаем допуска сервера...";
        if (deadline != 0)
        {
            ULONGLONG seconds = deadline > now ? (deadline - now + 999) / 1000 : 0;
            status = "Осталось " + to_string(seconds / 60) + " мин. " + to_string(seconds % 60) + " сек.";
        }
        string text = status + '\n' + line;
        if (text != previous)
        {
            CONSOLE_SCREEN_BUFFER_INFO info;
            if (!GetConsoleScreenBufferInfo(output, &info) || info.dwSize.Y < 3)
            {
                return;
            }
            if (previous.empty() || position.Y + 1 >= info.dwSize.Y)
            {
                cout.flush();
                DWORD written;
                if (!WriteConsoleW(output, L"\r\n\r\n", 4, &written, NULL) ||
                    !GetConsoleScreenBufferInfo(output, &info))
                {
                    return;
                }
                position = { 0, static_cast<SHORT>(info.dwCursorPosition.Y - 2) };
            }
            DrawLine(status, info.dwSize.X, position.Y);
            SHORT column = DrawLine(line, info.dwSize.X, position.Y + 1);
            SetConsoleCursorPosition(output, { column, static_cast<SHORT>(position.Y + 1) });
            previous = text;
            last_draw = now;
        }
    }
};


bool ReadServerLine(SOCKET socket, string& pending, string& line, long wait_ms)
{
    if (pending.find('\n') == string::npos)
    {
        int ready = WaitForSocket(socket, wait_ms);
        if (ready == 0)
        {
            return false;
        }
        char buffer[4096];
        int bytes = ready == SOCKET_ERROR ? SOCKET_ERROR : recv(socket, buffer, sizeof(buffer), 0);
        if (bytes <= 0)
        {
            throw string("Связь с сервером потеряна: сервер закрыл соединение без письма об отключении или недоступна сеть.\n");
        }
        pending.append(buffer, bytes);
    }

    size_t end = pending.find('\n');
    if (end > 8192 && pending.size() > 8192)
    {
        throw string("Слишком длинное сообщение от сервера.\n");
    }
    if (end == string::npos)
    {
        return false;
    }
    line = pending.substr(0, end);
    pending.erase(0, end + 1);
    const string prefix = "MAIL DISCONNECTED ";
    if (line.rfind(prefix, 0) == 0 && line != prefix + "DONE")
    {
        string reason = line.substr(prefix.size());
        if (reason == "TIMEOUT") throw string("Письмо сервера: время вышло, соединение закрыто по таймауту.\n");
        if (reason == "EXIT") throw string("Письмо сервера: соединение прервано командой exit.\n");
        throw string("Письмо сервера: соединение завершилось с ошибкой.\n");
    }
    return true;
}


string WaitForReply(SOCKET socket, string& pending, ConsoleStatus& status, ULONGLONG deadline, const string& hint, bool show_progress = false)
{
    string reply;
    do
    {
        status.Show(deadline, hint, show_progress);
    } while (!ReadServerLine(socket, pending, reply, 100));
    if (!show_progress) status.Clear();
    return reply;
}


void WaitForExchange(SOCKET socket, string& pending, ConsoleStatus& status, ULONGLONG deadline, int delay, const string& progress)
{
    ULONGLONG end = GetTickCount64() + delay;
    while (GetTickCount64() < end)
    {
        status.Show(deadline, progress, true);
        string reply;
        if (ReadServerLine(socket, pending, reply, 25))
        {
            throw string("Сервер завершил обмен раньше времени.\n");
        }
    }
}


string ReadInput(SOCKET socket, string& pending, ConsoleStatus& status, ULONGLONG deadline, const string& prompt)
{
    HANDLE input = GetStdHandle(STD_INPUT_HANDLE);
    DWORD mode;
    string value;
    if (!GetConsoleMode(input, &mode))
    {
        cout << prompt << flush;
        if (GetFileType(input) != FILE_TYPE_PIPE)
        {
            if (!getline(cin, value)) throw string("Ввод завершён.\n");
            return value;
        }
        while (true)
        {
            status.Show(deadline, "Ожидаем ввода.");
            string reply;
            if (ReadServerLine(socket, pending, reply, 25))
                throw string("Сервер завершил соединение во время ввода.\n");
            DWORD available = 0;
            if (!PeekNamedPipe(input, NULL, 0, NULL, &available, NULL))
                throw string("Ввод завершён.\n");
            if (available == 0) continue;
            char symbol;
            DWORD read;
            if (!ReadFile(input, &symbol, 1, &read, NULL) || read != 1)
                throw string("Ввод завершён.\n");
            if (symbol == '\n')
            {
                status.Clear();
                if (!value.empty() && value.back() == '\r') value.pop_back();
                return value;
            }
            if (value.size() >= 4096) throw string("Слишком длинная строка ввода.\n");
            value += symbol;
        }
    }

    while (true)
    {
        status.Show(deadline, prompt + value);
        string reply;
        if (ReadServerLine(socket, pending, reply, 25))
        {
            throw string("Сервер завершил соединение во время ввода.\n");
        }
        if (WaitForSingleObject(input, 0) != WAIT_OBJECT_0)
        {
            continue;
        }
        INPUT_RECORD event;
        DWORD read;
        if (!ReadConsoleInputW(input, &event, 1, &read) || read != 1)
        {
            throw string("Не удалось прочитать ввод.\n");
        }
        if (event.EventType != KEY_EVENT || !event.Event.KeyEvent.bKeyDown)
        {
            continue;
        }
        const KEY_EVENT_RECORD& key = event.Event.KeyEvent;
        if (key.wVirtualKeyCode == VK_RETURN)
        {
            status.Clear();
            cout << prompt << value << '\n';
            return value;
        }
        if (key.wVirtualKeyCode == VK_BACK)
        {
            if (!value.empty()) value.pop_back();
        }
        else if (key.uChar.UnicodeChar >= L' ' && key.uChar.UnicodeChar <= L'~' && value.size() < 63)
        {
            value += static_cast<char>(key.uChar.UnicodeChar);
        }
    }
}


ULONGLONG GetDeadline(const string& reply, const string& prefix)
{
    if (reply.rfind(prefix, 0) != 0)
    {
        throw string("Неожиданный ответ сервера: " + reply + ".\n");
    }
    return GetTickCount64() + GetNumber(reply.substr(prefix.size()), 1, 86400) * 1000ULL;
}


int main(int argc, char* argv[])
{
    InitConsole();
    bool discover = argc > 1 && string(argv[1]) == "--discover";
    int max_args = discover ? 5 : 4;
    if (argc > max_args || (argc > 1 && string(argv[1]) == "--help"))
    {
        cout << "ClientT.exe [имя DNS или IPv4] [TCP-порт] [пауза между запросами, мс]\n"
             << "По умолчанию: 127.0.0.1 2000 0\n"
             << "ClientT.exe --discover [UDP-порт] [позывной] [пауза между запросами, мс]\n";
        return argc > max_args ? 1 : 0;
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
        if (discover)
        {
            int udp_port = argc > 2 ? GetNumber(argv[2], 1, 65535) : 2000;
            string callsign = argc > 3 ? argv[3] : "FindServer";
            server_ip = FindServer(udp_port, callsign, server_port);
            if (argc > 4) delay = GetNumber(argv[4], 0, 120000);
        }
        else
        {
            if (argc > 1) server_ip = argv[1];
            if (argc > 2) server_port = GetNumber(argv[2], 1, 65535);
            if (argc > 3) delay = GetNumber(argv[3], 0, 120000);
        }
        SOCKADDR_IN server_address = {};
        int resolve_error = ResolveServer(server_ip, server_port, server_address);
        if (resolve_error != 0)
            throw SetErrorMessageText("Не удалось разрешить имя " + server_ip + ": ", resolve_error);
        client_socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
        if (client_socket == INVALID_SOCKET)
            throw SetErrorMessageText("Ошибка socket: ", WSAGetLastError());
        cout << "========== ClientT ==========\nПодключение к " << server_ip << ':' << server_port << "...\n";
        if (connect(client_socket, (sockaddr*)&server_address, sizeof(server_address)) == SOCKET_ERROR)
            throw SetErrorMessageText("Ошибка connect: ", WSAGetLastError());
        if (!EnableKeepAlive(client_socket))
            throw SetErrorMessageText("Ошибка настройки контроля соединения: ", WSAGetLastError());
        DWORD timeout = 2000;
        setsockopt(client_socket, SOL_SOCKET, SO_SNDTIMEO, (const char*)&timeout, sizeof(timeout));

        ConsoleStatus status;
        string pending;
        ULONGLONG deadline = GetDeadline(WaitForReply(client_socket, pending, status, 0,
            "TCP-соединение установлено. Ожидаем приёма клиента."), "MAIL CONNECTED ");
        cout << "Письмо сервера: подключение принято.\n"
             << "  echo — возврат отправленных сообщений\n"
             << "  time — дата и время сервера\n"
             << "  rand — случайное 32-битное целое число\n"
             << "Другой код — проверка ErrorInquiry.\n";
        string request_cmd;
        while (true)
        {
            request_cmd = ReadInput(client_socket, pending, status, deadline, "Сервис> ");
            if (!request_cmd.empty() && request_cmd.size() < 50 && request_cmd.find_first_of(" \t\r") == string::npos) break;
            cout << "Введите один код, например echo.\n";
        }
        bool known_service = request_cmd == "echo" || request_cmd == "time" || request_cmd == "rand";
        int messages_count = 0;
        if (known_service)
        {
            while (true)
            {
                string value = ReadInput(client_socket, pending, status, deadline, "Количество сообщений (1..1000000)> ");
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
        string answer = WaitForReply(client_socket, pending, status, deadline, "Ожидаем запуска сервиса.");
        if (answer == "ErrorInquiry")
        {
            cout << "Сервер ответил ErrorInquiry: сервис '" << request_cmd << "' не существует.\n";
            WaitForReply(client_socket, pending, status, deadline, "Ожидаем письма об отключении.");
        }
        else
        {
            deadline = GetDeadline(answer, "OK ");
            cout << "Сервис '" << request_cmd << "' запущен. Начинаем обмен.\n";
            auto start_time = chrono::steady_clock::now();
            string progress = "Ожидаем первого ответа.";
            for (int i = 1; i <= messages_count; i++)
            {
                string message = request_cmd == "echo" ? "Hello from Client " + to_string(i) : request_cmd;
                if (!SendLine(client_socket, message))
                    throw SetErrorMessageText("Ошибка отправки: ", WSAGetLastError());
                answer = WaitForReply(client_socket, pending, status, deadline, progress, true);
                if (answer.rfind("DATA ", 0) != 0)
                    throw string("Неожиданный ответ сервиса: " + answer + ".\n");
                progress = '[' + to_string(i) + '/' + to_string(messages_count) + "] " + answer.substr(5);
                if (!status.IsEnabled()) cout << progress << '\n' << flush;
                if (delay > 0 && i < messages_count)
                    WaitForExchange(client_socket, pending, status, deadline, delay, progress);
            }
            if (!SendLine(client_socket, "stop"))
                throw SetErrorMessageText("Ошибка завершения сервиса: ", WSAGetLastError());
            answer = WaitForReply(client_socket, pending, status, deadline, "Ожидаем письма об отключении.");
            if (answer != "MAIL DISCONNECTED DONE")
                throw string("Сервер не подтвердил завершение обмена.\n");
            double seconds = chrono::duration<double>(chrono::steady_clock::now() - start_time).count();
            cout << "Письмо сервера: обслуживание завершено. Ответов: " << messages_count << ", " << seconds << " с.\n";
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
