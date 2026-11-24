#define _WINSOCK_DEPRECATED_NO_WARNINGS
#pragma comment(lib, "WS2_32.lib")

#include <ctime>
#include <random>
#include <cstdint>
#include <cstdio>
#include "../ServerCommon.h"

#ifndef NOMINMAX
#define NOMINMAX
#endif


using namespace std;


typedef DWORD(WINAPI* tsServer)(LPVOID);


struct ServiceList
{
    char IdRequest[50];
    tsServer service;
};


DWORD WINAPI EchoServer(LPVOID lp_param);
DWORD WINAPI TimeServer(LPVOID lp_param);
DWORD WINAPI RandServer(LPVOID lp_param);

ServiceList TableService[] = {
    { "echo", EchoServer },
    { "time", TimeServer },
    { "rand", RandServer }
};


bool ReceiveServiceLine(Contact* contact, string& text)
{
    text.clear();
    while (ReadCounter(contact->finish_reason) == DONE)
    {
        int ready = WaitForSocket(contact->client_socket, 100);
        if (ready == 0)
        {
            continue;
        }
        char symbol;
        if (ready == SOCKET_ERROR || recv(contact->client_socket, &symbol, 1, 0) != 1)
        {
            return false;
        }
        if (symbol == '\n')
        {
            return true;
        }
        if (text.size() >= 4096)
        {
            return false;
        }
        text += symbol;
    }
    return false;
}


void PrintServiceEvent(Contact* contact, const char* event, const SYSTEMTIME& time)
{
    char message[256];
    sprintf_s(message, "[AcceptServer APC] %s '%s', %02u.%02u.%04u/%02u:%02u:%02u, клиент %s:%u", event, contact->service_name, time.wDay, time.wMonth, time.wYear, time.wHour, time.wMinute, time.wSecond, inet_ntoa(contact->params.sin_addr), ntohs(contact->params.sin_port));
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
        InterlockedCompareExchange(&contact->finish_reason, FAILED, DONE);
    }
    GetLocalTime(&contact->finish_time);
    QueueServiceEvent(contact, OnServiceStop);
    InterlockedExchange(&contact->status, FINISHED);
}


DWORD WINAPI EchoServer(LPVOID lp_param)
{
    Contact* contact = (Contact*)lp_param;
    StartService(contact);
    string request;
    while (ReceiveServiceLine(contact, request))
    {
        if (request == "stop" || !SendLine(contact->client_socket, "DATA " + request))
        {
            break;
        }
    }
    FinishService(contact, request != "stop");
    return 0;
}


DWORD WINAPI TimeServer(LPVOID lp_param)
{
    Contact* contact = (Contact*)lp_param;
    StartService(contact);
    string request;
    while (ReceiveServiceLine(contact, request) && request == "time")
    {
        time_t now = time(nullptr);
        tm local_time = {};
        char time_str[32];
        if (localtime_s(&local_time, &now) != 0 || strftime(time_str, sizeof(time_str), "%d.%m.%Y/%H:%M:%S", &local_time) == 0 || !SendLine(contact->client_socket, "DATA " + string(time_str)))
        {
            break;
        }
    }
    FinishService(contact, request != "stop");
    return 0;
}


DWORD WINAPI RandServer(LPVOID lp_param)
{
    Contact* contact = (Contact*)lp_param;
    StartService(contact);
    string request;
    try
    {
        mt19937 generator(random_device{}());
        uniform_int_distribution<int32_t> distribution(INT32_MIN, INT32_MAX);
        while (ReceiveServiceLine(contact, request) && request == "rand")
        {
            if (!SendLine(contact->client_socket, "DATA " + to_string(distribution(generator))))
            {
                break;
            }
        }
    }
    catch (...)
    {
        contact->log_message("[RandServer] Не удалось сформировать случайное число.");
    }
    FinishService(contact, request != "stop");
    return 0;
}


extern "C" __declspec(dllexport) HANDLE SSS(const char* id, LPVOID param)
{
    for (const auto& service : TableService)
    {
        if (strcmp(service.IdRequest, id) == 0)
        {
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