#define _WINSOCK_DEPRECATED_NO_WARNINGS

#pragma comment(lib, "WS2_32.lib")

#include <clocale>
#include <string>
#include <winsock2.h>
#include <windows.h>
#include <list>
#include <atomic>
#include <mutex>
#include <vector>
#include <iostream>
#ifndef NOMINMAX
#define NOMINMAX
#endif

// Общая структура подключения для сервера и ServiceLibrary.dll.
enum ContactStatus
{
    ACCEPT,
    SERVICED,
    FINISHED
};

struct Contact
{
    SOCKET client_socket = INVALID_SOCKET;
    SOCKADDR_IN params = {};
    int lprms = sizeof(params);
    volatile LONG status = ACCEPT;
    HANDLE h_accept_thread = NULL;
    char service_name[50] = {};
    HANDLE h_timer = NULL;
    HANDLE h_service_thread = NULL;
    volatile LONG is_error = FALSE;
    volatile LONG pending_apc = 0;
    bool timer_armed = false;
    int request_length = 0;
    ULONGLONG request_deadline = 0;
    SYSTEMTIME start_time = {};
    SYSTEMTIME finish_time = {};
    void (*log_message)(const std::string&) = nullptr;
};

inline LONG ReadCounter(volatile LONG& value)
{
    return InterlockedCompareExchange(&value, 0, 0);
}

inline void InitConsole()
{
    SetConsoleCP(CP_UTF8);
    SetConsoleOutputCP(CP_UTF8);
    setlocale(LC_ALL, ".UTF8");
}

inline bool SendAll(SOCKET socket, const char* data, int size)
{
    int sent = 0;
    while (sent < size)
    {
        int bytes = send(socket, data + sent, size - sent, 0);
        if (bytes <= 0)
        {
            return false;
        }
        sent += bytes;
    }
    return true;
}

inline bool SendLine(SOCKET socket, const std::string& text)
{
    std::string message = text + '\n';
    return SendAll(socket, message.data(), static_cast<int>(message.size()));
}

inline int ReceiveLine(SOCKET socket, std::string& text)
{
    text.clear();
    char symbol;
    while (true)
    {
        int bytes = recv(socket, &symbol, 1, 0);
        if (bytes <= 0)
        {
            return bytes == 0 && text.empty() ? 0 : -1;
        }
        if (symbol == '\n')
        {
            return 1;
        }
        text += symbol;
        if (text.size() > 4096)
        {
            return -1;
        }
    }
}

inline bool ReceiveAll(SOCKET socket, char* data, int size)
{
    int received = 0;
    while (received < size)
    {
        int bytes = recv(socket, data + received, size - received, 0);
        if (bytes <= 0)
        {
            return false;
        }
        received += bytes;
    }
    return true;
}

using namespace std;

int PORT = 2000;
HANDLE g_hAcceptServer = NULL;
HANDLE g_hAcceptEvent = NULL;
HANDLE g_hServerReady = NULL;
HANDLE g_hWorkersReady = NULL;
string DLL_NAME = "ServiceLibrary.dll";
string PIPE_NAME = "\\\\.\\pipe\\ServerPipe";
string CALLSIGN = "FindServer";
int UDP_PORT = 2000;


volatile LONG stat_total_connections = 0;
volatile LONG stat_total_rejections = 0;
volatile LONG stat_successful_connections = 0;
volatile LONG stat_active_connections = 0;


enum TalkersCommand {
    START,
    STOP,
    EXIT,
    STATISTICS,
    WAIT,
    SHUTDOWN,
    GETCOMMAND
};


list<Contact> contacts;
CRITICAL_SECTION scListContact;


typedef HANDLE(*SSS_Function)(const char*, LPVOID);
SSS_Function SSS = NULL;
VOID CALLBACK TimerAPCProc(LPVOID lpArgToCompletionRoutine, DWORD dwTimerLowValue, DWORD dwTimerHighValue);


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


void LogMessage(const string& message)
{
    static mutex log_mutex;
    lock_guard<mutex> lock(log_mutex);
    cout << message;
    if (message.empty() || message.back() != '\n')
    {
        cout << '\n';
    }
    cout.flush();
}

string ClientAddress(const Contact& contact)
{
    return string(inet_ntoa(contact.params.sin_addr)) + ":" + to_string(ntohs(contact.params.sin_port));
}

string GetStatistics()
{
    EnterCriticalSection(&scListContact);
    string result = "statistics\nВсего подключений:   " + to_string(ReadCounter(stat_total_connections)) +
        "\nОтказов/ошибок:      " + to_string(ReadCounter(stat_total_rejections)) +
        "\nУспешно завершено:   " + to_string(ReadCounter(stat_successful_connections)) +
        "\nАктивных клиентов:   " + to_string(ReadCounter(stat_active_connections));
    LeaveCriticalSection(&scListContact);
    return result;
}

void FinishWithoutService(Contact& contact, const string& reason)
{
    LogMessage("[DispatchServer] " + ClientAddress(contact) + ": " + reason);
    InterlockedExchange(&contact.is_error, TRUE);
    InterlockedExchange(&contact.status, FINISHED);
}

DWORD WINAPI ResponseServer(LPVOID lp_param)
{
    atomic<TalkersCommand>* command = (atomic<TalkersCommand>*)lp_param;
    SOCKET udp_socket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    SOCKADDR_IN udp_addr = {};
    udp_addr.sin_family = AF_INET;
    udp_addr.sin_port = htons(static_cast<u_short>(UDP_PORT));
    udp_addr.sin_addr.s_addr = INADDR_ANY;
    u_long nonblk = 1;
    if (udp_socket == INVALID_SOCKET ||
        bind(udp_socket, (sockaddr*)&udp_addr, sizeof(udp_addr)) == SOCKET_ERROR ||
        ioctlsocket(udp_socket, FIONBIO, &nonblk) == SOCKET_ERROR)
    {
        LogMessage(SetErrorMessageText("[ResponseServer] Ошибка запуска UDP: ", WSAGetLastError()));
        if (udp_socket != INVALID_SOCKET) closesocket(udp_socket);
        command->store(EXIT);
        return 1;
    }
    LogMessage("[ResponseServer] Поиск сервера: UDP " + to_string(UDP_PORT) + ", позывной '" + CALLSIGN + "'.");
    while (command->load() != EXIT)
    {
        char buffer[256];
        SOCKADDR_IN client_addr = {};
        int addr_len = sizeof(client_addr);
        int bytes = recvfrom(udp_socket, buffer, sizeof(buffer) - 1, 0, (sockaddr*)&client_addr, &addr_len);
        if (bytes > 0)
        {
            buffer[bytes] = '\0';
            if (strcmp(buffer, CALLSIGN.c_str()) == 0)
            {
                string invite = "Welcome to ConcurrentServer; TCP=" + to_string(PORT);
                if (sendto(udp_socket, invite.c_str(), static_cast<int>(invite.size()), 0,
                    (sockaddr*)&client_addr, addr_len) == SOCKET_ERROR)
                {
                    LogMessage(SetErrorMessageText("[ResponseServer] Ошибка ответа: ", WSAGetLastError()));
                }
                else
                {
                    LogMessage("[ResponseServer] Приглашение отправлено клиенту " + string(inet_ntoa(client_addr.sin_addr)) + ".");
                }
            }
        }
        else if (bytes == SOCKET_ERROR && WSAGetLastError() != WSAEWOULDBLOCK)
        {
            LogMessage(SetErrorMessageText("[ResponseServer] Ошибка recvfrom: ", WSAGetLastError()));
        }
        Sleep(20);
    }
    closesocket(udp_socket);
    LogMessage("[ResponseServer] Поток завершён.");
    return 0;
}

bool AcceptCycle(int accept_attempts, SOCKET server_socket)
{
    bool rc = false;
    while (accept_attempts-- > 0 && !rc)
    {
        Contact contact;
        contact.h_accept_thread = g_hAcceptServer;
        contact.log_message = LogMessage;
        contact.client_socket = accept(server_socket, (sockaddr*)&contact.params, &contact.lprms);
        if (contact.client_socket != INVALID_SOCKET)
        {
            u_long nonblk = 1;
            if (ioctlsocket(contact.client_socket, FIONBIO, &nonblk) == SOCKET_ERROR)
            {
                LogMessage(SetErrorMessageText("[AcceptServer] Ошибка ioctlsocket: ", WSAGetLastError()));
                closesocket(contact.client_socket);
                continue;
            }
            rc = true;
            contact.request_deadline = GetTickCount64() + 60000;
            EnterCriticalSection(&scListContact);
            contacts.push_front(contact);
            InterlockedIncrement(&stat_total_connections);
            InterlockedIncrement(&stat_active_connections);
            LeaveCriticalSection(&scListContact);
            LogMessage("[AcceptServer] Подключён клиент " + ClientAddress(contact) + ".");
            SetEvent(g_hAcceptEvent);
        }
        else if (WSAGetLastError() != WSAEWOULDBLOCK)
        {
            LogMessage(SetErrorMessageText("[AcceptServer] Ошибка accept: ", WSAGetLastError()));
        }
    }
    return rc;
}

void CommandsCycle(atomic<TalkersCommand>& cmd, SOCKET server_socket)
{
    bool is_accepting = false;
    while (cmd.load() != EXIT)
    {
        TalkersCommand current = cmd.load();
        switch (current)
        {
            case START:
            {
                is_accepting = true;
                LogMessage("[Управление] START: приём новых клиентов разрешён.");
                cmd.compare_exchange_strong(current, GETCOMMAND);
                break;
            }
            case STOP:
            {
                is_accepting = false;
                LogMessage("[Управление] STOP: приём новых клиентов приостановлен. Текущие клиенты продолжают работу.");
                cmd.compare_exchange_strong(current, GETCOMMAND);
                break;
            }
            case WAIT:
            case SHUTDOWN:
            {
                is_accepting = false;
                LogMessage("[Управление] " + string(current == WAIT ? "WAIT" : "SHUTDOWN") +
                    ": ожидаем завершения активных клиентов (" + to_string(ReadCounter(stat_active_connections)) + ").");
                while (ReadCounter(stat_active_connections) > 0 && cmd.load() != EXIT)
                {
                    SleepEx(20, TRUE);
                }
                if (cmd.load() != EXIT)
                {
                    if (current == SHUTDOWN)
                    {
                        LogMessage("[Управление] Клиенты обслужены. Завершаем сервер.");
                        cmd.store(EXIT);
                    }
                    else
                    {
                        is_accepting = true;
                        LogMessage("[Управление] WAIT выполнена. Все клиенты обслужены, приём возобновлён.");
                        cmd.store(GETCOMMAND);
                    }
                }
                break;
            }
            default:
                break;
        }
        if (is_accepting && cmd.load() != EXIT)
        {
            AcceptCycle(1, server_socket);
        }
        SleepEx(10, TRUE);
    }

    LogMessage("[Управление] EXIT: закрываем подключения и ожидаем завершения потоков.");
    EnterCriticalSection(&scListContact);
    for (auto& contact : contacts)
    {
        if (ReadCounter(contact.status) != FINISHED)
        {
            InterlockedExchange(&contact.is_error, TRUE);
            shutdown(contact.client_socket, SD_BOTH);
            if (ReadCounter(contact.status) == ACCEPT)
            {
                InterlockedExchange(&contact.status, FINISHED);
            }
        }
    }
    LeaveCriticalSection(&scListContact);
    SetEvent(g_hAcceptEvent);
    // AcceptServer остаётся alertable, пока уведомления DLL не будут выполнены.
    while (ReadCounter(stat_active_connections) > 0)
    {
        SleepEx(10, TRUE);
    }
    SleepEx(0, TRUE);
}

DWORD WINAPI AcceptServer(LPVOID lp_param)
{
    atomic<TalkersCommand>* command = (atomic<TalkersCommand>*)lp_param;
    g_hAcceptEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
    SOCKET server_socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    SOCKADDR_IN server_address = {};
    server_address.sin_family = AF_INET;
    server_address.sin_port = htons(static_cast<u_short>(PORT));
    server_address.sin_addr.s_addr = INADDR_ANY;
    u_long nonblk = 1;
    if (g_hAcceptEvent == NULL || server_socket == INVALID_SOCKET ||
        bind(server_socket, (sockaddr*)&server_address, sizeof(server_address)) == SOCKET_ERROR ||
        listen(server_socket, SOMAXCONN) == SOCKET_ERROR ||
        ioctlsocket(server_socket, FIONBIO, &nonblk) == SOCKET_ERROR)
    {
        LogMessage(SetErrorMessageText("[AcceptServer] Не удалось запустить TCP-сервер: ", WSAGetLastError()));
        if (server_socket != INVALID_SOCKET) closesocket(server_socket);
        command->store(EXIT);
        SetEvent(g_hServerReady);
        return 1;
    }
    LogMessage("[AcceptServer] TCP-порт " + to_string(PORT) + ". Ожидание клиентов.");
    SetEvent(g_hServerReady);
    WaitForSingleObject(g_hWorkersReady, INFINITE);
    CommandsCycle(*command, server_socket);
    closesocket(server_socket);
    LogMessage("[AcceptServer] Поток завершён.");
    return 0;
}

DWORD WINAPI GarbageCleaner(LPVOID lp_param)
{
    atomic<TalkersCommand>* command = (atomic<TalkersCommand>*)lp_param;
    while (command->load() != EXIT || ReadCounter(stat_active_connections) > 0)
    {
        EnterCriticalSection(&scListContact);
        for (auto it = contacts.begin(); it != contacts.end(); )
        {
            bool thread_finished = it->h_service_thread == NULL ||
                WaitForSingleObject(it->h_service_thread, 0) == WAIT_OBJECT_0;
            if (ReadCounter(it->status) == FINISHED && thread_finished &&
                ReadCounter(it->pending_apc) == 0 && !it->timer_armed)
            {
                LogMessage("[GarbageCleaner] Клиент " + ClientAddress(*it) +
                    (ReadCounter(it->is_error) ? ": завершён с отказом/ошибкой." : ": обслуживание завершено."));
                closesocket(it->client_socket);
                if (it->h_timer) CloseHandle(it->h_timer);
                if (it->h_service_thread) CloseHandle(it->h_service_thread);
                if (ReadCounter(it->is_error)) InterlockedIncrement(&stat_total_rejections);
                else InterlockedIncrement(&stat_successful_connections);
                InterlockedDecrement(&stat_active_connections);
                it = contacts.erase(it);
            }
            else
            {
                ++it;
            }
        }
        LeaveCriticalSection(&scListContact);
        Sleep(20);
    }
    LogMessage("[GarbageCleaner] Все подключения освобождены. Поток завершён.");
    return 0;
}

VOID CALLBACK TimerAPCProc(LPVOID lp_arg, DWORD, DWORD)
{
    Contact* contact = (Contact*)lp_arg;
    EnterCriticalSection(&scListContact);
    if (ReadCounter(contact->status) == SERVICED)
    {
        LogMessage("[Таймер] Сервис '" + string(contact->service_name) + "', клиент " + ClientAddress(*contact) +
            ": превышен лимит 60 секунд. Прерываем ожидание сокета.");
        InterlockedExchange(&contact->is_error, TRUE);
        // Вместо TerminateThread: recv/send завершаются, поток освобождает свои ресурсы.
        shutdown(contact->client_socket, SD_BOTH);
    }
    LeaveCriticalSection(&scListContact);
}

DWORD WINAPI DispatchServer(LPVOID lp_param)
{
    atomic<TalkersCommand>* command = (atomic<TalkersCommand>*)lp_param;
    while (command->load() != EXIT || ReadCounter(stat_active_connections) > 0)
    {
        WaitForSingleObjectEx(g_hAcceptEvent, 20, TRUE);
        EnterCriticalSection(&scListContact);
        for (auto& contact : contacts)
        {
            if (ReadCounter(contact.status) != ACCEPT) continue;
            bool request_ready = false;
            char symbol;
            // Читаем только первую строку, сохраняя остальной TCP-поток для сервиса.
            for (int i = 0; i < 50; i++)
            {
                int bytes = recv(contact.client_socket, &symbol, 1, 0);
                if (bytes == SOCKET_ERROR && WSAGetLastError() == WSAEWOULDBLOCK) break;
                if (bytes <= 0)
                {
                    FinishWithoutService(contact, "соединение закрыто до получения кода запроса.");
                    break;
                }
                if (symbol == '\n')
                {
                    request_ready = true;
                    break;
                }
                if (contact.request_length >= sizeof(contact.service_name) - 1)
                {
                    FinishWithoutService(contact, "слишком длинный код запроса.");
                    break;
                }
                contact.service_name[contact.request_length++] = symbol;
            }
            if (ReadCounter(contact.status) != ACCEPT) continue;
            if (!request_ready)
            {
                if (GetTickCount64() >= contact.request_deadline)
                    FinishWithoutService(contact, "код запроса не получен за 60 секунд.");
                continue;
            }
            LogMessage("[DispatchServer] " + ClientAddress(contact) + ": запрос '" + contact.service_name + "'.");
            u_long nonblk = 0;
            if (ioctlsocket(contact.client_socket, FIONBIO, &nonblk) == SOCKET_ERROR)
            {
                FinishWithoutService(contact, "не удалось переключить сокет в блокирующий режим.");
                continue;
            }
            DWORD timeout = 5000;
            setsockopt(contact.client_socket, SOL_SOCKET, SO_SNDTIMEO, (const char*)&timeout, sizeof(timeout));
            contact.h_service_thread = SSS(contact.service_name, &contact);
            if (contact.h_service_thread == NULL)
            {
                if (GetLastError() == ERROR_NOT_FOUND)
                {
                    SendLine(contact.client_socket, "ErrorInquiry");
                    FinishWithoutService(contact, "неизвестный сервис. Отправлен ErrorInquiry.");
                }
                else
                {
                    SendLine(contact.client_socket, "ErrorService");
                    FinishWithoutService(contact, "SSS не удалось создать обслуживающий поток.");
                }
                continue;
            }
            contact.h_timer = CreateWaitableTimer(NULL, FALSE, NULL);
            if (contact.h_timer == NULL)
            {
                TerminateThread(contact.h_service_thread, 1); // Поток ещё не запускался.
                SendLine(contact.client_socket, "ErrorService");
                FinishWithoutService(contact, "не удалось создать таймер.");
                continue;
            }
            InterlockedExchange(&contact.status, SERVICED);
            LARGE_INTEGER interval;
            interval.QuadPart = -600000000LL;
            if (!SetWaitableTimer(contact.h_timer, &interval, 0, TimerAPCProc, &contact, FALSE))
            {
                if (contact.h_service_thread)
                {
                    // Поток ещё не запускался и не владеет ресурсами.
                    TerminateThread(contact.h_service_thread, 1);
                }
                SendLine(contact.client_socket, "ErrorService");
                FinishWithoutService(contact, "не удалось запустить сервис/таймер.");
                continue;
            }
            contact.timer_armed = true;
            SetThreadPriority(contact.h_service_thread, THREAD_PRIORITY_NORMAL);
            if (!SendLine(contact.client_socket, "OK"))
            {
                InterlockedExchange(&contact.is_error, TRUE);
                shutdown(contact.client_socket, SD_BOTH);
            }
            ResumeThread(contact.h_service_thread);
        }
        // Отменяем таймеры в создавшем их потоке и выполняем уже поставленные APC.
        // Пока timer_armed=true, GarbageCleaner не может удалить Contact.
        vector<Contact*> finished_timers;
        for (auto& contact : contacts)
        {
            if (ReadCounter(contact.status) == FINISHED && contact.timer_armed)
            {
                CancelWaitableTimer(contact.h_timer);
                finished_timers.push_back(&contact);
            }
        }
        SleepEx(0, TRUE);
        for (Contact* contact : finished_timers) contact->timer_armed = false;
        LeaveCriticalSection(&scListContact);
    }
    LogMessage("[DispatchServer] Поток завершён.");
    return 0;
}

DWORD WINAPI ConsolePipe(LPVOID lp_param)
{
    atomic<TalkersCommand>* command = (atomic<TalkersCommand>*)lp_param;
    HANDLE h_pipe = CreateNamedPipeA(PIPE_NAME.c_str(), PIPE_ACCESS_DUPLEX,
        PIPE_TYPE_MESSAGE | PIPE_READMODE_MESSAGE | PIPE_NOWAIT, 1, 4096, 4096, 0, NULL);
    if (h_pipe == INVALID_HANDLE_VALUE)
    {
        LogMessage("[ConsolePipe] Ошибка создания канала: " + to_string(GetLastError()) + ".");
        command->store(EXIT);
        return 1;
    }
    LogMessage("[ConsolePipe] Канал " + PIPE_NAME + ". Команды вводятся в ClientNP.exe.");
    while (command->load() != EXIT)
    {
        if (!ConnectNamedPipe(h_pipe, NULL))
        {
            DWORD error = GetLastError();
            if (error != ERROR_PIPE_CONNECTED)
            {
                if (error == ERROR_NO_DATA) DisconnectNamedPipe(h_pipe);
                Sleep(20);
                continue;
            }
        }
        LogMessage("[ConsolePipe] Консоль управления подключена.");
        while (command->load() != EXIT)
        {
            DWORD available = 0;
            if (!PeekNamedPipe(h_pipe, NULL, 0, NULL, &available, NULL)) break;
            if (available == 0)
            {
                Sleep(20);
                continue;
            }
            char buffer[256] = {};
            DWORD bytes_read = 0;
            if (!ReadFile(h_pipe, buffer, sizeof(buffer) - 1, &bytes_read, NULL)) break;
            string input(buffer, bytes_read);
            string response;
            TalkersCommand next = GETCOMMAND;
            if (input == "start") next = START;
            else if (input == "stop") next = STOP;
            else if (input == "wait") next = WAIT;
            else if (input == "shutdown") next = SHUTDOWN;
            else if (input == "exit") next = EXIT;
            else if (input == "statistics") response = GetStatistics();
            else response = "nocmd";

            if (next != GETCOMMAND)
            {
                LogMessage("[ConsolePipe] Получена команда '" + input + "'.");
                TalkersCommand expected = GETCOMMAND;
                while (command->load() != EXIT &&
                    !command->compare_exchange_weak(expected, next))
                {
                    expected = GETCOMMAND;
                    Sleep(10);
                }
                if (next != EXIT && next != SHUTDOWN)
                {
                    while (command->load() != GETCOMMAND && command->load() != EXIT) Sleep(10);
                }
                response = input;
            }
            else if (input == "statistics")
            {
                LogMessage("[ConsolePipe] " + response);
            }
            else
            {
                LogMessage("[ConsolePipe] Неизвестная команда '" + input + "'. Ответ: nocmd.");
            }
            DWORD written = 0;
            if (!WriteFile(h_pipe, response.c_str(), static_cast<DWORD>(response.size()), &written, NULL) ||
                written != response.size()) break;
        }
        DisconnectNamedPipe(h_pipe);
        LogMessage("[ConsolePipe] Консоль управления отключена.");
    }
    CloseHandle(h_pipe);
    LogMessage("[ConsolePipe] Поток завершён.");
    return 0;
}

int ParseNumber(const char* text, int minimum, int maximum)
{
    string value(text);
    size_t end = 0;
    int number = stoi(value, &end);
    if (end != value.size() || number < minimum || number > maximum)
        throw string("Некорректный числовой параметр: " + value + ".\n");
    return number;
}

int main(int argc, char* argv[])
{
    InitConsole();
    if (argc > 6 || (argc > 1 && string(argv[1]) == "--help"))
    {
        cout << "Использование: ConcurrentServer.exe [TCP-порт] [DLL] [канал] [UDP-порт] [позывной]\n"
             << "По умолчанию: 2000 ServiceLibrary.dll \\\\.\\pipe\\ServerPipe 2000 FindServer\n";
        return argc > 6 ? 1 : 0;
    }
    try
    {
        if (argc > 1) PORT = ParseNumber(argv[1], 1, 65535);
        if (argc > 2) DLL_NAME = argv[2];
        if (argc > 3) PIPE_NAME = argv[3];
        if (argc > 4) UDP_PORT = ParseNumber(argv[4], 1, 65535);
        if (argc > 5) CALLSIGN = argv[5];
        if (CALLSIGN.empty() || CALLSIGN.size() > 255)
            throw string("Позывной должен содержать от 1 до 255 байт.\n");
    }
    catch (const string& error)
    {
        cerr << error;
        return 1;
    }
    catch (const exception&)
    {
        cerr << "Номер порта должен быть целым числом от 1 до 65535.\n";
        return 1;
    }

    WSADATA wsa_data;
    int wsa_error = WSAStartup(MAKEWORD(2, 2), &wsa_data);
    if (wsa_error != 0)
    {
        cerr << SetErrorMessageText("Ошибка WSAStartup: ", wsa_error);
        return 1;
    }
    HMODULE h_service_lib = LoadLibraryA(DLL_NAME.c_str());
    if (h_service_lib == NULL)
    {
        cerr << "Не удалось загрузить " << DLL_NAME << ". Код: " << GetLastError()
             << ". DLL должна соответствовать разрядности сервера.\n";
        WSACleanup();
        return 1;
    }
    SSS = (SSS_Function)GetProcAddress(h_service_lib, "SSS");
    if (SSS == NULL)
    {
        cerr << "В библиотеке не найдена экспортируемая функция SSS.\n";
        FreeLibrary(h_service_lib);
        WSACleanup();
        return 1;
    }

    LogMessage("========== ConcurrentServer ==========\nTCP: " + to_string(PORT) +
        " | UDP: " + to_string(UDP_PORT) + " | DLL: " + DLL_NAME +
        "\nУправление: ClientNP.exe (start, stop, wait, shutdown, statistics, exit).\n"
        "Клиенты: ClientT.exe [IPv4] [TCP-порт]. Сервисы: echo, time, rand.");
    InitializeCriticalSection(&scListContact);
    g_hServerReady = CreateEvent(NULL, TRUE, FALSE, NULL);
    g_hWorkersReady = CreateEvent(NULL, TRUE, FALSE, NULL);
    atomic<TalkersCommand> cmd_start(START);
    HANDLE threads[5] = {};
    bool started = g_hServerReady != NULL && g_hWorkersReady != NULL;
    if (started)
    {
        threads[0] = CreateThread(NULL, 0, AcceptServer, &cmd_start, CREATE_SUSPENDED, NULL);
        g_hAcceptServer = threads[0];
        started = threads[0] != NULL;
    }
    if (started)
    {
        SetThreadPriority(threads[0], THREAD_PRIORITY_HIGHEST);
        ResumeThread(threads[0]);
        WaitForSingleObject(g_hServerReady, INFINITE);
        started = cmd_start.load() != EXIT;
    }
    if (started)
    {
        threads[1] = CreateThread(NULL, 0, DispatchServer, &cmd_start, CREATE_SUSPENDED, NULL);
        threads[2] = CreateThread(NULL, 0, GarbageCleaner, &cmd_start, CREATE_SUSPENDED, NULL);
        threads[3] = CreateThread(NULL, 0, ConsolePipe, &cmd_start, CREATE_SUSPENDED, NULL);
        threads[4] = CreateThread(NULL, 0, ResponseServer, &cmd_start, CREATE_SUSPENDED, NULL);
        started = threads[1] && threads[2] && threads[3] && threads[4];
        if (!started)
        {
            LogMessage("[main] Не удалось создать все потоки. Завершаем сервер.");
            cmd_start.store(EXIT);
        }
        int priorities[] = { THREAD_PRIORITY_HIGHEST, THREAD_PRIORITY_ABOVE_NORMAL,
            THREAD_PRIORITY_LOWEST, THREAD_PRIORITY_NORMAL, THREAD_PRIORITY_ABOVE_NORMAL };
        for (int i = 1; i < 5; i++)
        {
            if (threads[i])
            {
                SetThreadPriority(threads[i], priorities[i]);
                ResumeThread(threads[i]);
            }
        }
    }
    if (!started) cmd_start.store(EXIT);
    if (g_hWorkersReady) SetEvent(g_hWorkersReady);
    if (threads[0]) WaitForSingleObject(threads[0], INFINITE);
    cmd_start.store(EXIT);
    if (g_hAcceptEvent) SetEvent(g_hAcceptEvent);
    for (int i = 0; i < 5; i++)
    {
        if (threads[i])
        {
            WaitForSingleObject(threads[i], INFINITE);
            CloseHandle(threads[i]);
        }
    }
    if (g_hAcceptEvent) CloseHandle(g_hAcceptEvent);
    if (g_hServerReady) CloseHandle(g_hServerReady);
    if (g_hWorkersReady) CloseHandle(g_hWorkersReady);
    DeleteCriticalSection(&scListContact);
    FreeLibrary(h_service_lib);
    WSACleanup();
    LogMessage("[main] Все потоки завершены. Библиотека отключена. Сервер остановлен.");
    return started ? 0 : 1;
}
