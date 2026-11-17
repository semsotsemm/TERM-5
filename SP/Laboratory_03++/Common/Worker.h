#pragma once

#include "Console.h"
#include <random>

using WorkerOperation = BOOL (*)(HT::HTHANDLE*, int32_t);

inline void PrintOperation(const char* app, int32_t key)
{
    SYSTEMTIME time;
    GetLocalTime(&time);
    char timestamp[16];
    sprintf_s(timestamp, "%02u:%02u:%02u", static_cast<unsigned>(time.wHour),
        static_cast<unsigned>(time.wMinute), static_cast<unsigned>(time.wSecond));
    std::cout << '[' << timestamp << "] " << app << " | ключ=" << key << " | ";
}

inline int RunWorker(int argc, wchar_t* argv[], const char* app, WorkerOperation operation)
{
    InitConsole();
    char file_name[512];
    if (argc != 2 || !FileNameToUtf8(argv[1], file_name))
    {
        std::cerr << "Использование: " << app << ".exe <имя файла>\n";
        return 1;
    }

    HT::HTHANDLE* ht = HT::Open(file_name);
    if (ht == nullptr)
    {
        PrintStorageError();
        return 1;
    }

    if (ht->MaxKeyLength < sizeof(int32_t) || ht->MaxPayloadLength < sizeof(int32_t))
    {
        std::cerr << "Размер ключа и данных должен быть не меньше 4 байт.\n";
        HT::Close(ht);
        return 1;
    }
    if (!SetUpStopEvent())
    {
        std::cerr << "Не удалось установить обработчик Ctrl+C.\n";
        HT::Close(ht);
        return 1;
    }

    std::cout << app << ": подключено к " << file_name << ". Для выхода нажмите Ctrl+C." << std::endl;
    std::mt19937 generator(GetTickCount() ^ GetCurrentProcessId());
    std::uniform_int_distribution<int32_t> keys(0, 49);

    while (WaitForSingleObject(stop_event, 1000) == WAIT_TIMEOUT)
    {
        int32_t key = keys(generator);
        PrintOperation(app, key);
        if (!operation(ht, key))
        {
            DWORD code = ::GetLastError();
            std::cout << HT::GetLastError(ht) << std::endl;
            if (code == ERROR_NOT_READY) break;
        }
    }

    CloseStopEvent();
    HT::Close(ht);
    std::cout << app << ": работа завершена." << std::endl;
    return 0;
}
