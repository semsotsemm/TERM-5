#pragma once

#include <Windows.h>
#include <cstdint>
#include <ctime>

#ifdef OS11_HTAPI_EXPORTS
#define HT_API __declspec(dllexport)
#else
#define HT_API __declspec(dllimport)
#endif

namespace HT
{
    // Каждый процесс хранит свой блок управления в памяти.
    struct HTHANDLE
    {
        int Capacity = 0;
        int SecSnapshotInterval = 0;
        int MaxKeyLength = 0;
        int MaxPayloadLength = 0;
        char FileName[512] = {};
        HANDLE File = INVALID_HANDLE_VALUE;
        HANDLE FileMapping = NULL;
        LPVOID Addr = nullptr;
        char LastErrorMessage[512] = {};
        time_t lastsnaptime = 0;

        HANDLE Mutex = NULL;
        HANDLE ReadyEvent = NULL;
        uint64_t Generation = 0;
        wchar_t ReadyEventName[96] = {};
    };

    struct HT_API Element
    {
        Element();
        Element(const void* key, int keylength);
        Element(const void* key, int keylength, const void* payload, int payloadlength);
        Element(Element* oldelement, const void* newpayload, int newpayloadlength);

        const void* key;
        int keylength;
        const void* payload;
        int payloadlength;
    };

    // Create возвращает закрытое для других процессов хранилище. Start открывает общий доступ.
    // Имена файлов передаются в UTF-8.
    HT_API HTHANDLE* Create(int capacity, int snapshot_interval, int max_key_length,
        int max_payload_length, const char* file_name);
    HT_API HTHANDLE* Start(const char* file_name);
    HT_API HTHANDLE* Open(const char* file_name);
    HT_API BOOL Snap(const HTHANDLE* ht);
    HT_API BOOL Close(const HTHANDLE* ht);

    HT_API BOOL Insert(const HTHANDLE* ht, const Element* element);
    HT_API BOOL Delete(const HTHANDLE* ht, const Element* element);
    // Копия живёт в памяти вызывающего процесса. Освобождается через FreeElement.
    HT_API Element* Get(const HTHANDLE* ht, const Element* element);
    HT_API void FreeElement(Element* element);
    HT_API BOOL Update(const HTHANDLE* ht, const Element* oldelement,
        const void* new_payload, int new_payload_length);
    HT_API BOOL Increment(const HTHANDLE* ht, const Element* element, int32_t* result);

    HT_API char* GetLastError(HTHANDLE* ht);
    HT_API void print(const Element* element);
}
