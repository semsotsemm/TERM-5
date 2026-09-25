#include "HT.h"
#include <iostream>

namespace HT {

    HTHANDLE::HTHANDLE() {
        Capacity = 0;
        SecSnapshotInterval = 0;
        MaxKeyLength = 0;
        MaxPayloadLength = 0;
        memset(FileName, 0, sizeof(FileName));
        File = NULL;
        FileMapping = NULL;
        Addr = NULL;
        lastsnaptime = time(NULL);
        memset(LastErrorMessage, 0, sizeof(LastErrorMessage));
    }

    HTHANDLE::HTHANDLE(int Capacity, int SecSnapshotInterval, int MaxKeyLength, int MaxPayloadLength, const char FileName[512]) {
        this->Capacity = Capacity;
        this->SecSnapshotInterval = SecSnapshotInterval;
        this->MaxKeyLength = MaxKeyLength;
        this->MaxPayloadLength = MaxPayloadLength;
        strncpy_s(this->FileName, 512, FileName, _TRUNCATE); // Безопасное копирование
        File = NULL;
        FileMapping = NULL;
        Addr = NULL;
        lastsnaptime = time(NULL);
        memset(LastErrorMessage, 0, sizeof(LastErrorMessage));
    }

    Element::Element() : key(nullptr), keylength(0), payload(nullptr), payloadlength(0) {}

    Element::Element(const void* key, int keylength)
        : key(key), keylength(keylength), payload(nullptr), payloadlength(0) {
    }

    Element::Element(const void* key, int keylength, const void* payload, int payloadlength)
        : key(key), keylength(keylength), payload(payload), payloadlength(payloadlength) {
    }

    Element::Element(Element* oldelement, const void* newpayload, int newpayloadlength)
        : key(oldelement->key), keylength(oldelement->keylength), payload(newpayload), payloadlength(newpayloadlength) {
    }

    HTHANDLE* Create(int Capacity, int SecSnapshotInterval, int MaxKeyLength, int MaxPayloadLength, const char FileName[512]) {
        HTHANDLE* ht = new HTHANDLE(Capacity, SecSnapshotInterval, MaxKeyLength, MaxPayloadLength, FileName);

        size_t elementSize = MaxKeyLength + MaxPayloadLength + sizeof(int) * 2 + sizeof(bool);
        size_t totalSize = sizeof(HTHANDLE) + (Capacity * elementSize);

        ht->File = CreateFileA(FileName, GENERIC_READ | GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
        if (ht->File == INVALID_HANDLE_VALUE) {
            delete ht;
            return nullptr;
        }

        ht->FileMapping = CreateFileMappingA(ht->File, NULL, PAGE_READWRITE, 0, static_cast<DWORD>(totalSize), NULL);
        if (!ht->FileMapping) {
            CloseHandle(ht->File);
            delete ht;
            return nullptr;
        }

        ht->Addr = MapViewOfFile(ht->FileMapping, FILE_MAP_ALL_ACCESS, 0, 0, totalSize);
        if (!ht->Addr) {
            CloseHandle(ht->FileMapping);
            CloseHandle(ht->File);
            delete ht;
            return nullptr;
        }

        memcpy(ht->Addr, ht, sizeof(HTHANDLE));

        return ht;
    }

    HTHANDLE* Open(const char FileName[512]) {
        HTHANDLE* ht = new HTHANDLE();
        strncpy_s(ht->FileName, 512, FileName, _TRUNCATE);

        ht->File = CreateFileA(FileName, GENERIC_READ | GENERIC_WRITE, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
        if (ht->File == INVALID_HANDLE_VALUE) {
            delete ht;
            return nullptr;
        }

        ht->FileMapping = CreateFileMappingA(ht->File, NULL, PAGE_READWRITE, 0, 0, NULL);

        if (!ht->FileMapping) {
            CloseHandle(ht->File);
            delete ht;
            return nullptr;
        }

        ht->Addr = MapViewOfFile(ht->FileMapping, FILE_MAP_ALL_ACCESS, 0, 0, 0);

        if (ht->Addr) {
            HTHANDLE* mappedMeta = static_cast<HTHANDLE*>(ht->Addr);
            ht->Capacity = mappedMeta->Capacity;
            ht->MaxKeyLength = mappedMeta->MaxKeyLength;
            ht->MaxPayloadLength = mappedMeta->MaxPayloadLength;
        }
        else {
            CloseHandle(ht->FileMapping);
            CloseHandle(ht->File);
            delete ht;
            return nullptr;
        }

        return ht;
    }

    BOOL Snap(const HTHANDLE* hthandle) {
        if (!hthandle || !hthandle->Addr) return FALSE;
        if (FlushViewOfFile(hthandle->Addr, 0)) {
            const_cast<HTHANDLE*>(hthandle)->lastsnaptime = time(NULL);
            return TRUE;
        }
        return FALSE;
    }

    BOOL Close(const HTHANDLE* hthandle) {
        if (!hthandle) return FALSE;
        Snap(hthandle);
        if (hthandle->Addr) UnmapViewOfFile(hthandle->Addr);
        if (hthandle->FileMapping) CloseHandle(hthandle->FileMapping);
        if (hthandle->File) CloseHandle(hthandle->File);
        delete hthandle;
        return TRUE;
    }

    BOOL Insert(const HTHANDLE* hthandle, const Element* element) { return TRUE; }
    Element* Get(const HTHANDLE* hthandle, const Element* element) {
        return new Element(element->key, element->keylength, "payload", 8);
    }
    BOOL Update(const HTHANDLE* hthandle, const Element* oldelement, const void* newpayload, int newpayloadlength) { return TRUE; }
    BOOL Delete(const HTHANDLE* hthandle, const Element* element) { return TRUE; }

    char* GetLastError(HTHANDLE* ht) {
        return ht->LastErrorMessage;
    }

    void print(const Element* element) {
        if (element && element->key && element->payload) {
            std::cout << "Key: " << (char*)element->key << " Payload: " << (char*)element->payload << std::endl;
        }
    }
}