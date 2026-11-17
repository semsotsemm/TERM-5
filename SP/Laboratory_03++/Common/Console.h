#pragma once

#include "../OS11_HTAPI/HT.h"
#include <climits>
#include <clocale>
#include <cstdio>
#include <iostream>

inline HANDLE stop_event = NULL;

inline void InitConsole()
{
    SetConsoleCP(CP_UTF8);
    SetConsoleOutputCP(CP_UTF8);
    setlocale(LC_ALL, ".UTF8");
}

inline BOOL WINAPI StopOnCtrlC(DWORD signal)
{
    if (signal != CTRL_C_EVENT && signal != CTRL_BREAK_EVENT) return FALSE;

    SetEvent(stop_event);
    return TRUE;
}

inline bool SetUpStopEvent()
{
    stop_event = CreateEventW(nullptr, TRUE, FALSE, nullptr);
    if (stop_event == NULL) return false;

    if (!SetConsoleCtrlHandler(StopOnCtrlC, TRUE))
    {
        CloseHandle(stop_event);
        stop_event = NULL;
        return false;
    }
    return true;
}

inline void CloseStopEvent()
{
    SetConsoleCtrlHandler(StopOnCtrlC, FALSE);
    CloseHandle(stop_event);
    stop_event = NULL;
}

inline bool FileNameToUtf8(const wchar_t* name, char (&utf8_name)[512])
{
    return WideCharToMultiByte(CP_UTF8, WC_ERR_INVALID_CHARS, name, -1, utf8_name,
        static_cast<int>(sizeof(utf8_name)), nullptr, nullptr) != 0;
}

inline void PrintStorage(const char* action, const HT::HTHANDLE* ht)
{
    std::cout << "HT-Storage " << action << " filename=" << ht->FileName
        << ", snapshotinterval=" << ht->SecSnapshotInterval << '\n'
        << "capacity=" << ht->Capacity << ", maxkeylength=" << ht->MaxKeyLength
        << ", maxdatalength=" << ht->MaxPayloadLength << std::endl;
}

inline void PrintStorageError(HT::HTHANDLE* ht = nullptr)
{
    DWORD code = ::GetLastError();
    std::cerr << HT::GetLastError(ht) << " Код Windows: " << code << std::endl;
}
