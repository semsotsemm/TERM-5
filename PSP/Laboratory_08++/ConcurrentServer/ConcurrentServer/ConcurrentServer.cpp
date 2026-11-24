#define _WINSOCK_DEPRECATED_NO_WARNINGS

#pragma comment(lib, "WS2_32.lib")

#include "../ServerCommon.h"
#include <list>
#include <atomic>
#include <mutex>
#include <vector>
#include <iostream>
#include <sstream>

#ifndef NOMINMAX
#define NOMINMAX
#endif


using namespace std;


int PORT = 2000;
int UDP_PORT = 2000;
int SERVICE_TIMEOUT = 180;

string DLL_NAME = "ServiceLibrary.dll";

string PIPE_NAME = "\\\\.\\pipe\\ServerPipe";

string CALLSIGN = "FindServer";

HANDLE g_hAcceptServer = NULL;
HANDLE g_hAcceptEvent = NULL;
HANDLE g_hServerReady = NULL;
HANDLE g_hWorkersReady = NULL;

CRITICAL_SECTION scListContact;

volatile LONG stat_total_connections = 0;
volatile LONG stat_total_rejections = 0;
volatile LONG stat_timeouts = 0;
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

struct ServiceSchedule
{
    string service_name;
    int start_minutes;
    int end_minutes;
};

vector<ServiceSchedule> service_schedules;
mutex schedule_mutex;

typedef HANDLE(*SSS_Function)(const char*, LPVOID);
SSS_Function SSS = NULL;

VOID CALLBACK TimerAPCProc(LPVOID lpArgToCompletionRoutine, DWORD dwTimerLowValue, DWORD dwTimerHighValue);


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


bool ParseScheduleTime(const string& text, int& minutes)
{
    if (text.size() != 5 || text[2] != ':')
    {
        return false;
    }
    for (int i = 0; i < 5; i++)
    {
        if (i != 2 && (text[i] < '0' || text[i] > '9'))
        {
            return false;
        }
    }
    int hours = (text[0] - '0') * 10 + text[1] - '0';
    int minute_part = (text[3] - '0') * 10 + text[4] - '0';
    if (hours > 23 || minute_part > 59)
    {
        return false;
    }
    minutes = hours * 60 + minute_part;
    return true;
}


string FormatScheduleTime(int minutes)
{
    int hours = minutes / 60;
    int minute_part = minutes % 60;
    return string(hours < 10 ? "0" : "") + to_string(hours) + ':' + (minute_part < 10 ? "0" : "") + to_string(minute_part);
}


string SetServiceSchedule(istringstream& command_stream)
{
    ServiceSchedule schedule;
    string start_time;
    string end_time;
    string extra_argument;
    if (!(command_stream >> schedule.service_name >> start_time >> end_time) || command_stream >> extra_argument)
    {
        return "Формат: SCHEDULE <echo|time|rand> <HH:MM> <HH:MM>. Например: SCHEDULE echo 08:00 12:00.";
    }
    if (schedule.service_name != "echo" && schedule.service_name != "time" && schedule.service_name != "rand")
    {
        return "Неизвестный сервис. Доступны: echo, time, rand.";
    }
    if (!ParseScheduleTime(start_time, schedule.start_minutes) ||
        !ParseScheduleTime(end_time, schedule.end_minutes))
    {
        return "Некорректное время. Используйте HH:MM: часы 00..23, минуты 00..59.";
    }
    if (schedule.start_minutes == schedule.end_minutes)
    {
        return "Начало и конец расписания должны отличаться.";
    }

    lock_guard<mutex> lock(schedule_mutex);
    bool schedule_found = false;
    for (auto& current_schedule : service_schedules)
    {
        if (current_schedule.service_name == schedule.service_name)
        {
            current_schedule = schedule;
            schedule_found = true;
            break;
        }
    }
    if (!schedule_found)
    {
        service_schedules.push_back(schedule);
    }
    return "Расписание сервиса '" + schedule.service_name + "': с " + start_time + " до " + end_time + " по местному времени сервера. Начало включено, конец не включён.";
}


bool IsServiceAvailable(const string& service_name, string& rejection_message)
{
    lock_guard<mutex> lock(schedule_mutex);
    for (const auto& schedule : service_schedules)
    {
        if (schedule.service_name != service_name)
        {
            continue;
        }

        SYSTEMTIME current_time;
        GetLocalTime(&current_time);
        int current_minutes = current_time.wHour * 60 + current_time.wMinute;
        bool allowed;
        if (schedule.start_minutes < schedule.end_minutes)
        {
            allowed = current_minutes >= schedule.start_minutes && current_minutes < schedule.end_minutes;
        }
        else
        {
            allowed = current_minutes >= schedule.start_minutes || current_minutes < schedule.end_minutes;
        }
        if (!allowed)
        {
            rejection_message = "Сервис '" + service_name + "' недоступен по расписанию. Время сервера: " + FormatScheduleTime(current_minutes) + ". Разрешено с " + FormatScheduleTime(schedule.start_minutes) +  " до " + FormatScheduleTime(schedule.end_minutes) + ".";
        }
        return allowed;
    }
    return true;
}


string ClientAddress(const Contact& contact)
{
    return string(inet_ntoa(contact.params.sin_addr)) + ":" + to_string(ntohs(contact.params.sin_port));
}


string GetStatistics()
{
    EnterCriticalSection(&scListContact);
    string result = "statistics\nВсего подключений:   " + to_string(ReadCounter(stat_total_connections));
    result += "\nОтказов/ошибок:      " + to_string(ReadCounter(stat_total_rejections));
    result += "\nУспешно завершено:   " + to_string(ReadCounter(stat_successful_connections));
    result += "\nЗавершено по таймауту: " + to_string(ReadCounter(stat_timeouts));
    result += "\nАктивных клиентов:   " + to_string(ReadCounter(stat_active_connections));
    LeaveCriticalSection(&scListContact);
    return result;
}


void FinishWithoutService(Contact& contact, const string& reason, FinishReason result = FAILED)
{
    LogMessage("[DispatchServer] " + ClientAddress(contact) + ": " + reason);
    InterlockedExchange(&contact.finish_reason, result);
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
    bool is_bind = (bind(udp_socket, (sockaddr*)&udp_addr, sizeof(udp_addr)) == SOCKET_ERROR);
    bool is_options = (ioctlsocket(udp_socket, FIONBIO, &nonblk) == SOCKET_ERROR);
    if (udp_socket == INVALID_SOCKET || is_bind || is_options)
    {
        LogMessage(SetErrorMessageText("[ResponseServer] Ошибка запуска UDP: ", WSAGetLastError()));
        if (udp_socket != INVALID_SOCKET)
        {
            closesocket(udp_socket);
        }
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
                if (sendto(udp_socket, invite.c_str(), static_cast<int>(invite.size()), 0, (sockaddr*)&client_addr, addr_len) == SOCKET_ERROR)
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
            contact.request_deadline = GetTickCount64() + SERVICE_TIMEOUT * 1000ULL;
            if (!EnableKeepAlive(contact.client_socket) ||
                !SendLine(contact.client_socket, "MAIL CONNECTED " + to_string(SERVICE_TIMEOUT)))
            {
                FinishWithoutService(contact, "не удалось отправить письмо о подключении.");
            }
           
            EnterCriticalSection(&scListContact);
            
            contacts.push_front(contact);
            InterlockedIncrement(&stat_total_connections);
            InterlockedIncrement(&stat_active_connections);
            
            LeaveCriticalSection(&scListContact);
            
            LogMessage("[Письмо] Подключён клиент " + ClientAddress(contact) + ".");
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
                LogMessage("[Управление] " + string(current == WAIT ? "WAIT" : "SHUTDOWN") + ": ожидаем завершения активных клиентов (" + to_string(ReadCounter(stat_active_connections)) + ").");
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
        if (is_accepting && cmd.load() == GETCOMMAND)
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
            InterlockedCompareExchange(&contact.finish_reason, SERVER_EXIT, DONE);
            if (ReadCounter(contact.status) == ACCEPT)
            {
                InterlockedExchange(&contact.status, FINISHED);
            }
        }
    }
    LeaveCriticalSection(&scListContact);
    SetEvent(g_hAcceptEvent);
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

    bool is_event_error = (g_hAcceptEvent == NULL);
    bool is_socket_error = (server_socket == INVALID_SOCKET);
    bool is_bind_error = (bind(server_socket, (sockaddr*)&server_address, sizeof(server_address)) == SOCKET_ERROR);
    bool is_listen_error = (listen(server_socket, SOMAXCONN) == SOCKET_ERROR);
    bool is_ioctl_error = (ioctlsocket(server_socket, FIONBIO, &nonblk) == SOCKET_ERROR);

    if (is_event_error || is_socket_error || is_bind_error || is_listen_error || is_ioctl_error)
    {
        LogMessage(SetErrorMessageText("[AcceptServer] Не удалось запустить TCP-сервер: ", WSAGetLastError()));
        if (server_socket != INVALID_SOCKET)
        {
            closesocket(server_socket);
        }

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
            bool thread_finished = it->h_service_thread == NULL || WaitForSingleObject(it->h_service_thread, 0) == WAIT_OBJECT_0;
            if (ReadCounter(it->status) == FINISHED && thread_finished && ReadCounter(it->pending_apc) == 0 && !it->timer_armed)
            {
                LONG result = ReadCounter(it->finish_reason);
                string reason;
                switch (result)
                {
                    case TIMED_OUT: reason = "TIMEOUT"; break;
                    case SERVER_EXIT: reason = "EXIT"; break;
                    case DONE: reason = "DONE"; break;
                    default: reason = "ERROR"; break;
                }
                LogMessage("[Письмо] Отключён клиент " + ClientAddress(*it) + ": " + reason + ".");
                SendLine(it->client_socket, "MAIL DISCONNECTED " + reason);
                shutdown(it->client_socket, SD_SEND);
                closesocket(it->client_socket);
                
                if (it->h_timer)
                {
                    CloseHandle(it->h_timer);
                }
                
                if (it->h_service_thread)
                {
                    CloseHandle(it->h_service_thread);
                }
                
                if (result == TIMED_OUT)
                {
                    InterlockedIncrement(&stat_timeouts);
                }
                else if (result == DONE)
                {
                    InterlockedIncrement(&stat_successful_connections);
                }
                else
                {
                    InterlockedIncrement(&stat_total_rejections);
                }

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
        if (InterlockedCompareExchange(&contact->finish_reason, TIMED_OUT, DONE) == DONE)
        {
            LogMessage("[Таймер] Сервис '" + string(contact->service_name) + "', клиент " + ClientAddress(*contact) + ": превышен лимит " + to_string(SERVICE_TIMEOUT) + " сек.");
        }
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
            if (ReadCounter(contact.status) != ACCEPT)
            {
                continue;
            }
            if (command->load() == EXIT)
            {
                continue;
            }
            if (GetTickCount64() >= contact.request_deadline)
            {
                FinishWithoutService(contact, "истекло время на выбор сервиса.", TIMED_OUT);
                continue;
            }

            bool request_ready = false;
            char symbol;
            for (int i = 0; i < 50; i++)
            {
                int bytes = recv(contact.client_socket, &symbol, 1, 0);
                if (bytes == SOCKET_ERROR && WSAGetLastError() == WSAEWOULDBLOCK)
                {
                    break;
                }
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

            if (ReadCounter(contact.status) != ACCEPT)
            {
                continue;
            }
            
            if (!request_ready)
            {
                continue;
            }
            
            LogMessage("[DispatchServer] " + ClientAddress(contact) + ": запрос '" + contact.service_name + "'.");
            u_long nonblk = 0;

            if (ioctlsocket(contact.client_socket, FIONBIO, &nonblk) == SOCKET_ERROR)
            {
                FinishWithoutService(contact, "не удалось переключить сокет в блокирующий режим.");
                continue;
            }
            
            DWORD timeout = 1000;
            setsockopt(contact.client_socket, SOL_SOCKET, SO_SNDTIMEO, (const char*)&timeout, sizeof(timeout));

            string rejection_message;
            if (!IsServiceAvailable(contact.service_name, rejection_message))
            {
                SendLine(contact.client_socket, "ErrorSchedule " + rejection_message);
                FinishWithoutService(contact, rejection_message);
                continue;
            }
            
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
                TerminateThread(contact.h_service_thread, 1);
                SendLine(contact.client_socket, "ErrorService");
                FinishWithoutService(contact, "не удалось создать таймер.");
                continue;
            }
            InterlockedExchange(&contact.status, SERVICED);
            
            LARGE_INTEGER interval;
            interval.QuadPart = -10000000LL * SERVICE_TIMEOUT;
            if (!SetWaitableTimer(contact.h_timer, &interval, 0, TimerAPCProc, &contact, FALSE))
            {
                if (contact.h_service_thread)
                {
                    TerminateThread(contact.h_service_thread, 1);
                }
                SendLine(contact.client_socket, "ErrorService");
                FinishWithoutService(contact, "не удалось запустить сервис/таймер.");
                continue;
            }

            contact.timer_armed = true;
            SetThreadPriority(contact.h_service_thread, THREAD_PRIORITY_NORMAL);
            if (!SendLine(contact.client_socket, "OK " + to_string(SERVICE_TIMEOUT)))
            {
                InterlockedCompareExchange(&contact.finish_reason, FAILED, DONE);
            }

            ResumeThread(contact.h_service_thread);
        }

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

        for (Contact* contact : finished_timers)
        {
            contact->timer_armed = false;
        }
        
        LeaveCriticalSection(&scListContact);
    }

    LogMessage("[DispatchServer] Поток завершён.");
    return 0;
}


DWORD WINAPI ConsolePipe(LPVOID lp_param)
{
    atomic<TalkersCommand>* command = (atomic<TalkersCommand>*)lp_param;

    DWORD open_mode = PIPE_ACCESS_DUPLEX;
    DWORD pipe_mode = PIPE_TYPE_MESSAGE | PIPE_READMODE_MESSAGE | PIPE_NOWAIT; 
    HANDLE h_pipe = CreateNamedPipeA(PIPE_NAME.c_str(), open_mode, pipe_mode, 1, 4096, 4096, 0, NULL);
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
                if (error == ERROR_NO_DATA)
                {
                    DisconnectNamedPipe(h_pipe);
                }
                Sleep(20);
                continue;
            }
        }

        LogMessage("[ConsolePipe] Консоль управления подключена.");
        while (command->load() != EXIT)
        {
            DWORD available = 0;
            if (!PeekNamedPipe(h_pipe, NULL, 0, NULL, &available, NULL))
            {
                break;
            }
            if (available == 0)
            {
                Sleep(20);
                continue;
            }

            char buffer[256] = {};
            DWORD bytes_read = 0;
            if (!ReadFile(h_pipe, buffer, sizeof(buffer) - 1, &bytes_read, NULL))
            {
                break;
            }

            string input(buffer, bytes_read);
            string response;
            istringstream command_stream(input);
            string command_name;
            command_stream >> command_name;
            bool is_schedule = command_name == "SCHEDULE" || command_name == "schedule";

            TalkersCommand next = GETCOMMAND;
            if (is_schedule)
            {
                response = SetServiceSchedule(command_stream);
            }
            else if (input == "start")
            {
                next = START;
            }
            else if (input == "stop")
            {
                next = STOP;
            }
            else if (input == "wait")
            {
                next = WAIT;
            }
            else if (input == "shutdown")
            {
                next = SHUTDOWN;
            }
            else if (input == "exit")
            {
                next = EXIT;
            }
            else if (input == "statistics")
            {
                response = GetStatistics();
            }
            else
            {
                response = "nocmd";
            }

            if (next != GETCOMMAND)
            {
                LogMessage("[ConsolePipe] Получена команда '" + input + "'.");
                TalkersCommand expected = GETCOMMAND;
                
                while (command->load() != EXIT && !command->compare_exchange_weak(expected, next))
                {
                    expected = GETCOMMAND;
                    Sleep(10);
                }

                if (next != EXIT && next != SHUTDOWN)
                {
                    while (command->load() != GETCOMMAND && command->load() != EXIT)
                    {
                        Sleep(10);
                    }
                }

                response = input;
            }
            else if (input == "statistics" || is_schedule)
            {
                LogMessage("[ConsolePipe] " + response);
            }
            else
            {
                LogMessage("[ConsolePipe] Неизвестная команда '" + input + "'. Ответ: nocmd.");
            }
            DWORD written = 0;
            if (!WriteFile(h_pipe, response.c_str(), static_cast<DWORD>(response.size()), &written, NULL) || written != response.size())
            {
                break;
            }
            if (next == EXIT || next == SHUTDOWN)
            {
                ULONGLONG deadline = GetTickCount64() + 1000;
                while (GetTickCount64() < deadline && PeekNamedPipe(h_pipe, NULL, 0, NULL, NULL, NULL))
                {
                    Sleep(20);
                }
                CloseHandle(h_pipe);
                LogMessage("[ConsolePipe] Консоль управления отключена.");
                return 0;
            }
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
    {
        throw string("Некорректный числовой параметр: " + value + ".\n");
    }
    return number;
}


int main(int argc, char* argv[])
{
    InitConsole();

    if (argc > 7 || (argc > 1 && string(argv[1]) == "--help"))
    {
        cout << "Использование: ConcurrentServer.exe [TCP-порт] [DLL] [Имя канала] [UDP-порт] [Широковещательный позывной] [Таймаут, сек]\n" << "По умолчанию: [2000] [ServiceLibrary.dll] [\\\\.\\pipe\\ServerPipe] [2000] [FindServer] [180]\n";
        return argc > 7 ? 1 : 0;
    }
    try
    {
        if (argc > 1)
        {
            PORT = ParseNumber(argv[1], 1, 65535);
        }
        if (argc > 2)
        {
            DLL_NAME = argv[2];
        }
        if (argc > 3)
        {
            PIPE_NAME = argv[3];
        }
        if (argc > 4)
        {
            UDP_PORT = ParseNumber(argv[4], 1, 65535);
        }
        if (argc > 5)
        {
            CALLSIGN = argv[5];
        }
        if (argc > 6)
        {
            SERVICE_TIMEOUT = ParseNumber(argv[6], 1, 86400);
        }
        if (CALLSIGN.empty() || CALLSIGN.size() > 255)
        {
            throw string("Позывной должен содержать от 1 до 255 байт.\n");
        }
    }
    catch (const string& error)
    {
        cerr << error;
        return 1;
    }
    catch (const exception&)
    {
        cerr << "Порты: 1..65535. Таймаут: целое число от 1 до 86400 секунд.\n";
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
        cerr << "Не удалось загрузить " << DLL_NAME << ". Код: " << GetLastError() << ". DLL должна соответствовать разрядности сервера.\n";
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

    LogMessage("========== ConcurrentServer ==========\nTCP: " + to_string(PORT) + " | UDP: " + to_string(UDP_PORT) + " | DLL: " + DLL_NAME + " | Таймаут: " + to_string(SERVICE_TIMEOUT) + " сек." + "\nУправление: ClientNP.exe (start, stop, wait, shutdown, statistics, exit, SCHEDULE <ServiceName> <HH:MM> <HH:MM>).\n" + "Клиенты: ClientT.exe [имя или IPv4] [TCP-порт]. Сервисы: echo, time, rand.");
   
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

        int priorities[] = { THREAD_PRIORITY_HIGHEST, THREAD_PRIORITY_ABOVE_NORMAL, THREAD_PRIORITY_LOWEST, THREAD_PRIORITY_NORMAL, THREAD_PRIORITY_ABOVE_NORMAL };
        for (int i = 1; i < 5; i++)
        {
            if (threads[i])
            {
                SetThreadPriority(threads[i], priorities[i]);
                ResumeThread(threads[i]);
            }
        }
    }
    
    if (!started)
    {
        cmd_start.store(EXIT);
    }
    if (g_hWorkersReady)
    {
        SetEvent(g_hWorkersReady);
    }
    if (threads[0])
    {
        WaitForSingleObject(threads[0], INFINITE);
    }
    
    cmd_start.store(EXIT);
    
    if (g_hAcceptEvent)
    {
        SetEvent(g_hAcceptEvent);
    }
    
    for (int i = 0; i < 5; i++)
    {
        if (threads[i])
        {
            WaitForSingleObject(threads[i], INFINITE);
            CloseHandle(threads[i]);
        }
    }
    if (g_hAcceptEvent)
    {
        CloseHandle(g_hAcceptEvent);
    }
    if (g_hServerReady)
    {
        CloseHandle(g_hServerReady);
    }
    if (g_hWorkersReady)
    {
        CloseHandle(g_hWorkersReady);
    }
    
    DeleteCriticalSection(&scListContact);
    FreeLibrary(h_service_lib);
    WSACleanup();
    LogMessage("[main] Все потоки завершены. Библиотека отключена. Сервер остановлен.");
    return started ? 0 : 1;
}