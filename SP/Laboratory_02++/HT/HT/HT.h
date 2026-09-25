#pragma once
#include <windows.h>

namespace HT
{
    // Блок управления HT
    struct HTHANDLE
    {
        HTHANDLE();
        HTHANDLE(int Capacity, int SecSnapshotInterval, int MaxKeyLength, int MaxPayloadLength, const char FileName[512]);

        int Capacity;                  // емкость хранилища в количестве элементов
        int SecSnapshotInterval;       // периодичность сохранения в сек.
        int MaxKeyLength;              // максимальная длина ключа
        int MaxPayloadLength;          // максимальная длина данных
        char FileName[512];            // имя файла
        HANDLE File;                   // File HANDLE != 0, если файл открыт
        HANDLE FileMapping;            // Mapping File HANDLE != 0, если mapping создан
        LPVOID Addr;                   // Addr != NULL, если mapview выполнен
        char LastErrorMessage[512];    // сообщение об последней ошибке или 0x00
        time_t lastsnaptime;           // дата последнего snap'a (time())
    };

    // Элемент
    struct Element
    {
        Element();
        Element(const void* key, int keylength);                                            // for Get
        Element(const void* key, int keylength, const void* payload, int payloadlength);    // for Insert
        Element(Element* oldelement, const void* newpayload, int newpayloadlength);         // for update

        const void* key;               // значение ключа
        int keylength;                 // размер ключа
        const void* payload;           // данные
        int payloadlength;             // размер данных
    };

    // HT API
    HTHANDLE* Create(int Capacity, int SecSnapshotInterval, int MaxKeyLength, int MaxPayloadLength, const char FileName[512]);
    HTHANDLE* Open(const char FileName[512]);
    BOOL Snap(const HTHANDLE* hthandle);
    BOOL Close(const HTHANDLE* hthandle);
    BOOL Insert(const HTHANDLE* hthandle, const Element* element);
    BOOL Delete(const HTHANDLE* hthandle, const Element* element);
    Element* Get(const HTHANDLE* hthandle, const Element* element);
    BOOL Update(const HTHANDLE* hthandle, const Element* oldelement, const void* newpayload, int newpayloadlength);
    char* GetLastError(HTHANDLE* ht);
    void print(const Element* element);
}