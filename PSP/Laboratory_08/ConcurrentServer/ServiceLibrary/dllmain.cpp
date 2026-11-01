#define _WINSOCK_DEPRECATED_NO_WARNINGS
#pragma comment(lib, "WS2_32.lib")

#include "../ServerCommon.h"
#include <ctime>
#include <random>
#include <cstdint>
#include <cstdio>

using namespace std;

void PrintServiceEvent(Contact* contact, const char* event, const SYSTEMTIME& time)
{
    char message[256];
    sprintf_s(message, "[AcceptServer APC] %s '%s', %02u.%02u.%04u/%02u:%02u:%02u, клиент %s:%u",
        event, contact->service_name, time.wDay, time.wMonth, time.wYear,
        time.wHour, time.wMinute, time.wSecond, inet_ntoa(contact->params.sin_addr),
        ntohs(contact->params.sin_port));
    contact->log_message(message);
}

VOID CALLBACK OnServiceStart(ULONG_PTR dwParam)
{
    Contact* contact = (Contact*)dwParam;
    PrintServiceEvent(contact, "СТАРТ", contact->start_time);
    InterlockedDecrement(&contact->pending_apc);
}

VOID CALLBACK OnServiceStop(ULONG_PTR dwParam)
{
    Contact* contact = (Contact*)dwParam;
    PrintServiceEvent(contact, "СТОП", contact->finish_time);
    if (contact->h_timer != NULL)
    {
        CancelWaitableTimer(contact->h_timer);
    }
    InterlockedDecrement(&contact->pending_apc);
}

void QueueServiceEvent(Contact* contact, PAPCFUNC callback)
{
    InterlockedIncrement(&contact->pending_apc);
    if (!QueueUserAPC(callback, contact->h_accept_thread, (ULONG_PTR)contact))
    {
        InterlockedDecrement(&contact->pending_apc);
        contact->log_message("[ServiceLibrary] Не удалось поставить уведомление в очередь APC.");
    }
}

void StartService(Contact* contact)
{
    GetLocalTime(&contact->start_time);
    QueueServiceEvent(contact, OnServiceStart);
}

void FinishService(Contact* contact, bool is_error)
{
    if (is_error)
    {
        InterlockedExchange(&contact->is_error, TRUE);
    }
    GetLocalTime(&contact->finish_time);
    QueueServiceEvent(contact, OnServiceStop);
    // Сокет и дескрипторы освобождает GarbageCleaner после выхода потока и APC.
    InterlockedExchange(&contact->status, FINISHED);
}

DWORD WINAPI EchoServer(LPVOID lp_param)
{
    Contact* contact = (Contact*)lp_param;
    StartService(contact);
    char buffer[4096];
    bool is_error = false;
    while (true)
    {
        int bytes = recv(contact->client_socket, buffer, sizeof(buffer), 0);
        if (bytes == 0)
        {
            break;
        }
        if (bytes == SOCKET_ERROR || !SendAll(contact->client_socket, buffer, bytes))
        {
            is_error = true;
            break;
        }
    }
    FinishService(contact, is_error);
    return 0;
}

DWORD WINAPI TimeServer(LPVOID lp_param)
{
    Contact* contact = (Contact*)lp_param;
    StartService(contact);
    string request;
    int result = 0;
    bool is_error = false;
    while ((result = ReceiveLine(contact->client_socket, request)) == 1 && request == "time")
    {
        time_t now = time(nullptr);
        tm local_time = {};
        char time_str[32];
        if (localtime_s(&local_time, &now) != 0 ||
            strftime(time_str, sizeof(time_str), "%d.%m.%Y/%H:%M:%S", &local_time) == 0 ||
            !SendLine(contact->client_socket, time_str))
        {
            is_error = true;
            break;
        }
    }
    FinishService(contact, is_error || result < 0);
    return 0;
}

DWORD WINAPI RandServer(LPVOID lp_param)
{
    Contact* contact = (Contact*)lp_param;
    StartService(contact);
    string request;
    int result = 0;
    bool is_error = false;
    try
    {
        // Отдельный генератор на поток: без общего srand и seed по секундам.
        mt19937 generator(random_device{}());
        uniform_int_distribution<int32_t> distribution(INT32_MIN, INT32_MAX);
        while ((result = ReceiveLine(contact->client_socket, request)) == 1 && request == "rand")
        {
            int32_t number = distribution(generator);
            // Intel-формат: ровно четыре байта, младший байт первым (Windows x86/x64).
            if (!SendAll(contact->client_socket, reinterpret_cast<const char*>(&number), sizeof(number)))
            {
                is_error = true;
                break;
            }
        }
        is_error = is_error || result < 0;
    }
    catch (...)
    {
        is_error = true;
        contact->log_message("[RandServer] Не удалось сформировать случайное число.");
    }
    FinishService(contact, is_error);
    return 0;
}

typedef DWORD(WINAPI* tsServer)(LPVOID);

struct ServiceList
{
    char IdRequest[50];
    tsServer service;
};

ServiceList TableService[] = {
    { "echo", EchoServer },
    { "time", TimeServer },
    { "rand", RandServer }
};

extern "C" __declspec(dllexport) HANDLE SSS(const char* id, LPVOID param)
{
    for (const auto& service : TableService)
    {
        if (strcmp(service.IdRequest, id) == 0)
        {
            // Диспетчер настроит таймер и подтвердит запрос до ResumeThread.
            return CreateThread(NULL, 0, service.service, param, CREATE_SUSPENDED, NULL);
        }
    }
    SetLastError(ERROR_NOT_FOUND);
    return NULL;
}

BOOL APIENTRY DllMain(HMODULE, DWORD, LPVOID)
{
    return TRUE;
}
