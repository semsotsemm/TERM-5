#include "HT.h"

#include <climits>
#include <cstdio>
#include <cstring>
#include <new>

namespace HT
{
    namespace
    {
        const uint32_t FILE_MAGIC = 0x31315448; 
        const uint32_t FILE_VERSION = 1;

        
        struct StorageHeader
        {
            uint32_t Magic;
            uint32_t Version;
            int32_t Capacity;
            int32_t SnapshotInterval;
            int32_t MaxKeyLength;
            int32_t MaxPayloadLength;
            uint64_t Generation;
        };

        
        enum SlotState
        {
            SLOT_EMPTY,
            SLOT_BUSY,
            SLOT_DELETED
        };

        
        struct SlotHeader
        {
            int32_t State;
            int32_t KeyLength;
            int32_t PayloadLength;
        };

        
        static_assert(sizeof(StorageHeader) == 32);
        static_assert(sizeof(SlotHeader) == 12);

        
        thread_local char last_error[512] = {};

        
        BOOL SetError(const HTHANDLE* ht, const char* message, DWORD code)
        {
            strcpy_s(last_error, message);
            if (ht != nullptr)
            {
                strcpy_s(const_cast<HTHANDLE*>(ht)->LastErrorMessage, message);
            }
            ::SetLastError(code);
            return FALSE;
        }

        
        void ClearError(const HTHANDLE* ht)
        {
            SetError(ht, "", ERROR_SUCCESS);
        }

        
        class MutexLock
        {
            HANDLE mutex;

        public:
            DWORD Error = ERROR_SUCCESS;

            explicit MutexLock(HANDLE handle) : mutex(handle)
            {
                if (mutex == NULL) return;

                DWORD result = WaitForSingleObject(mutex, INFINITE);
                if (result != WAIT_OBJECT_0 && result != WAIT_ABANDONED)
                {
                    Error = ::GetLastError();
                    mutex = NULL;
                }
            }

            ~MutexLock()
            {
                if (mutex != NULL)
                {
                    DWORD code = ::GetLastError();
                    ReleaseMutex(mutex);
                    ::SetLastError(code);
                }
            }

            MutexLock(const MutexLock&) = delete;
            MutexLock& operator=(const MutexLock&) = delete;
        };

        
        bool GetStorageSize(int capacity, int interval, int max_key, int max_payload, uint64_t& size)
        {
            if (capacity <= 0 || interval <= 0 || max_key <= 0 || max_payload <= 0)
            {
                return false;
            }

            uint64_t slot_size = sizeof(SlotHeader) + static_cast<uint64_t>(max_key) + max_payload;
            if (slot_size > (INT64_MAX - sizeof(StorageHeader)) / static_cast<uint64_t>(capacity))
            {
                return false;
            }

            size = sizeof(StorageHeader) + static_cast<uint64_t>(capacity) * slot_size;
            return size <= SIZE_MAX;
        }

        
        bool GetFilePath(const char* file_name, wchar_t (&path)[512])
        {
            if (file_name == nullptr || *file_name == '\0' || strlen(file_name) >= 512)
            {
                ::SetLastError(ERROR_INVALID_NAME);
                return false;
            }

            wchar_t name[512];
            if (MultiByteToWideChar(CP_UTF8, MB_ERR_INVALID_CHARS, file_name, -1, name, 512) == 0)
            {
                return false;
            }

            DWORD length = GetFullPathNameW(name, 512, path, nullptr);
            if (length >= 512) ::SetLastError(ERROR_FILENAME_EXCED_RANGE);
            return length > 0 && length < 512;
        }

        
        void ReleaseStorage(const HTHANDLE* ht)
        {
            if (ht->ReadyEvent != NULL) CloseHandle(ht->ReadyEvent);
            if (ht->Addr != nullptr) UnmapViewOfFile(ht->Addr);
            if (ht->FileMapping != NULL) CloseHandle(ht->FileMapping);
            if (ht->File != INVALID_HANDLE_VALUE) CloseHandle(ht->File);
            if (ht->Mutex != NULL) CloseHandle(ht->Mutex);
            delete ht;
        }

        
        HTHANDLE* OpenError(HTHANDLE* ht, const char* message, DWORD code, const wchar_t* created_file = nullptr)
        {
            if (ht != nullptr) ReleaseStorage(ht);
            if (created_file != nullptr) DeleteFileW(created_file);
            SetError(nullptr, message, code);
            return nullptr;
        }

        
        StorageHeader* Header(const HTHANDLE* ht)
        {
            return static_cast<StorageHeader*>(ht->Addr);
        }

        
        void ReadParameters(HTHANDLE* ht)
        {
            const StorageHeader* header = Header(ht);
            ht->Capacity = header->Capacity;
            ht->SecSnapshotInterval = header->SnapshotInterval;
            ht->MaxKeyLength = header->MaxKeyLength;
            ht->MaxPayloadLength = header->MaxPayloadLength;
            ht->Generation = header->Generation;
        }

        
        BOOL SaveStorage(const HTHANDLE* ht)
        {
            if (!FlushViewOfFile(ht->Addr, 0) || !FlushFileBuffers(ht->File))
            {
                return SetError(ht, "Не удалось сохранить хранилище в файл.", ::GetLastError());
            }

            const_cast<HTHANDLE*>(ht)->lastsnaptime = time(nullptr);
            ClearError(ht);
            return TRUE;
        }

        
        bool CheckRunning(const HTHANDLE* ht)
        {
            if (ht->ReadyEvent != NULL || ht->Mutex == NULL)
            {
                return true;
            }
            if (Header(ht)->Generation == ht->Generation)
            {
                HANDLE event = OpenEventW(SYNCHRONIZE, FALSE, ht->ReadyEventName);
                if (event != NULL)
                {
                    CloseHandle(event);
                    return true;
                }
            }

            SetError(ht, "OS11_START завершил работу. Хранилище отключено.", ERROR_NOT_READY);
            return false;
        }

        
        BOOL AttachStorage(HTHANDLE* ht, bool start)
        {
            BY_HANDLE_FILE_INFORMATION info = {};
            if (!GetFileInformationByHandle(ht->File, &info))
            {
                return SetError(ht, "Не удалось получить идентификатор файла.", ::GetLastError());
            }

            wchar_t name[64];
            swprintf_s(name, L"Local\\OS11_HT_%08lX_%08lX_%08lX", info.dwVolumeSerialNumber, info.nFileIndexHigh, info.nFileIndexLow);
            wchar_t mutex_name[96];
            wchar_t mapping_name[96];
            swprintf_s(mutex_name, L"%ls_mutex", name);
            swprintf_s(mapping_name, L"%ls_mapping", name);
            swprintf_s(ht->ReadyEventName, L"%ls_ready", name);

            ht->Mutex = CreateMutexW(nullptr, FALSE, mutex_name);
            if (ht->Mutex == NULL)
            {
                return SetError(ht, "Не удалось создать mutex хранилища.", ::GetLastError());
            }

            MutexLock lock(ht->Mutex);
            if (lock.Error != ERROR_SUCCESS)
            {
                return SetError(ht, "Не удалось заблокировать хранилище.", lock.Error);
            }

            HANDLE event = OpenEventW(SYNCHRONIZE, FALSE, ht->ReadyEventName);
            DWORD event_error = ::GetLastError();
            if (event != NULL)
            {
                CloseHandle(event);
                if (start)
                {
                    return SetError(ht, "Это хранилище уже запущено.", ERROR_ALREADY_EXISTS);
                }
            }
            else if (!start || event_error != ERROR_FILE_NOT_FOUND)
            {
                return SetError(ht, "Хранилище недоступно. Сначала запустите OS11_START.", ERROR_NOT_READY);
            }

            LARGE_INTEGER file_size = {};
            if (!GetFileSizeEx(ht->File, &file_size))
            {
                return SetError(ht, "Не удалось определить размер файла.", ::GetLastError());
            }
            if (file_size.QuadPart < sizeof(StorageHeader) ||
                static_cast<uint64_t>(file_size.QuadPart) > SIZE_MAX)
            {
                return SetError(ht, "Неверный размер файла HT-хранилища.", ERROR_INVALID_DATA);
            }

            if (start)
            {
                ht->FileMapping = CreateFileMappingW(ht->File, nullptr, PAGE_READWRITE, 0, 0, mapping_name);
            }
            else
            {
                ht->FileMapping = OpenFileMappingW(FILE_MAP_WRITE, FALSE, mapping_name);
            }
            if (ht->FileMapping == NULL)
            {
                return SetError(ht, "Не удалось открыть образ файла.", ::GetLastError());
            }

            ht->Addr = MapViewOfFile(ht->FileMapping, FILE_MAP_WRITE, 0, 0, static_cast<SIZE_T>(file_size.QuadPart));
            if (ht->Addr == nullptr)
            {
                return SetError(ht, "Не удалось отобразить файл в память.", ::GetLastError());
            }

            StorageHeader* header = Header(ht);
            uint64_t expected_size = 0;
            if (header->Magic != FILE_MAGIC || header->Version != FILE_VERSION || !GetStorageSize(header->Capacity, header->SnapshotInterval, header->MaxKeyLength, header->MaxPayloadLength, expected_size) || expected_size != static_cast<uint64_t>(file_size.QuadPart))
            {
                return SetError(ht, "Неверный формат файла. Создайте хранилище через OS11_CREATE.", ERROR_INVALID_DATA);
            }

            if (start)
            {
                ht->ReadyEvent = CreateEventW(nullptr, TRUE, TRUE, ht->ReadyEventName);
                if (ht->ReadyEvent == NULL)
                {
                    return SetError(ht, "Не удалось открыть доступ к хранилищу.", ::GetLastError());
                }
                // Клиенты предыдущего запуска всё ещё могут держать отображение.
                ++header->Generation;
            }

            ReadParameters(ht);
            ClearError(ht);
            return TRUE;
        }

        
        HTHANDLE* OpenStorage(const char* file_name, bool start)
        {
            wchar_t path[512];
            if (!GetFilePath(file_name, path))
            {
                return OpenError(nullptr, "Некорректное имя файла.", ::GetLastError());
            }

            HTHANDLE* ht = new (std::nothrow) HTHANDLE;
            if (ht == nullptr)
            {
                return OpenError(nullptr, "Недостаточно памяти.", ERROR_NOT_ENOUGH_MEMORY);
            }

            strcpy_s(ht->FileName, file_name);
            ht->File = CreateFileW(path, GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE,
                nullptr, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
            if (ht->File == INVALID_HANDLE_VALUE)
            {
                return OpenError(ht, "Не удалось открыть файл хранилища.", ::GetLastError());
            }

            if (!AttachStorage(ht, start))
            {
                DWORD code = ::GetLastError();
                ReleaseStorage(ht);
                ::SetLastError(code);
                return nullptr;
            }

            return ht;
        }

        
        bool ValidKey(const HTHANDLE* ht, const Element* element)
        {
            if (ht != nullptr && ht->Addr != nullptr && element != nullptr && element->key != nullptr && element->keylength > 0 && element->keylength <= ht->MaxKeyLength)
            {
                return true;
            }

            SetError(ht, "Неверный ключ или размер ключа.", ERROR_INVALID_PARAMETER);
            return false;
        }

        
        bool ValidPayload(const HTHANDLE* ht, const void* data, int length)
        {
            if (length >= 0 && length <= ht->MaxPayloadLength && (data != nullptr || length == 0))
            {
                return true;
            }

            SetError(ht, "Неверные данные или размер данных.", ERROR_INVALID_PARAMETER);
            return false;
        }


        BYTE* GetSlot(const HTHANDLE* ht, int index)
        {
            size_t slot_size = sizeof(SlotHeader) + static_cast<size_t>(ht->MaxKeyLength) + ht->MaxPayloadLength;
            return static_cast<BYTE*>(ht->Addr) + sizeof(StorageHeader) + slot_size * index;
        }


        BYTE* KeyAddress(BYTE* slot)
        {
            return slot + sizeof(SlotHeader);
        }


        BYTE* PayloadAddress(const HTHANDLE* ht, BYTE* slot)
        {
            return KeyAddress(slot) + ht->MaxKeyLength;
        }


        SlotHeader ReadSlot(BYTE* slot)
        {
            SlotHeader info;
            memcpy(&info, slot, sizeof(info));
            return info;
        }


        uint32_t HashKey(const Element* element)
        {
            const BYTE* key = static_cast<const BYTE*>(element->key);
            uint32_t hash = 2166136261u;
            for (int i = 0; i < element->keylength; ++i)
            {
                hash = (hash ^ key[i]) * 16777619u;
            }
            return hash;
        }


        BYTE* FindSlot(const HTHANDLE* ht, const Element* element, BYTE*& free_slot)
        {
            free_slot = nullptr;
            int index = static_cast<int>(HashKey(element) % static_cast<uint32_t>(ht->Capacity));
            for (int i = 0; i < ht->Capacity; ++i)
            {
                BYTE* slot = GetSlot(ht, index);
                SlotHeader info = ReadSlot(slot);

                if (info.State == SLOT_EMPTY)
                {
                    if (free_slot == nullptr) free_slot = slot;
                    break;
                }
                if (info.State == SLOT_DELETED)
                {
                    if (free_slot == nullptr) free_slot = slot;
                }
                else if (info.State == SLOT_BUSY && info.KeyLength == element->keylength &&
                    memcmp(KeyAddress(slot), element->key, element->keylength) == 0)
                {
                    return slot;
                }

                if (++index == ht->Capacity) index = 0;
            }
            return nullptr;
        }


        BYTE* FindExistingSlot(const HTHANDLE* ht, const Element* element)
        {
            BYTE* free_slot;
            BYTE* slot = FindSlot(ht, element, free_slot);
            if (slot == nullptr)
            {
                SetError(ht, "Ключ не найден.", ERROR_NOT_FOUND);
            }
            return slot;
        }


        void WritePayload(const HTHANDLE* ht, BYTE* slot, const void* data, int length)
        {
            if (length != 0) memmove(PayloadAddress(ht, slot), data, length);
        }


        void PrintBytes(const void* data, int size)
        {
            const BYTE* bytes = static_cast<const BYTE*>(data);
            for (int i = 0; i < size; ++i)
            {
                printf("%02X ", static_cast<unsigned>(bytes[i]));
            }
        }
    }

    Element::Element() : Element(nullptr, 0)
    {
    }


    Element::Element(const void* key, int keylength) : Element(key, keylength, nullptr, 0)
    {
    }


    Element::Element(const void* key, int keylength, const void* payload, int payloadlength)
        : key(key), keylength(keylength), payload(payload), payloadlength(payloadlength)
    {
    }


    Element::Element(Element* oldelement, const void* newpayload, int newpayloadlength) 
        : Element(oldelement->key, oldelement->keylength, newpayload, newpayloadlength)
    {
    }


    HTHANDLE* Create(int capacity, int snapshot_interval, int max_key_length, int max_payload_length, const char* file_name)
    {
        uint64_t size = 0;
        if (!GetStorageSize(capacity, snapshot_interval, max_key_length, max_payload_length, size))
        {
            return OpenError(nullptr, "Допустимы только положительные параметры и поддерживаемый размер файла.",
                ERROR_INVALID_PARAMETER);
        }

        wchar_t path[512];
        if (!GetFilePath(file_name, path))
        {
            return OpenError(nullptr, "Некорректное имя файла.", ::GetLastError());
        }

        HTHANDLE* ht = new (std::nothrow) HTHANDLE;
        if (ht == nullptr)
        {
            return OpenError(nullptr, "Недостаточно памяти.", ERROR_NOT_ENOUGH_MEMORY);
        }

        strcpy_s(ht->FileName, file_name);
        ht->File = CreateFileW(path, GENERIC_READ | GENERIC_WRITE, 0, nullptr, CREATE_NEW, FILE_ATTRIBUTE_NORMAL, NULL);
        if (ht->File == INVALID_HANDLE_VALUE)
        {
            DWORD code = ::GetLastError();
            return OpenError(ht, code == ERROR_FILE_EXISTS ? "Файл уже существует." :
                "Не удалось создать файл хранилища.", code);
        }

        LARGE_INTEGER position;
        position.QuadPart = static_cast<LONGLONG>(size);
        if (!SetFilePointerEx(ht->File, position, nullptr, FILE_BEGIN) || !SetEndOfFile(ht->File))
        {
            return OpenError(ht, "Не удалось задать размер файла.", ::GetLastError(), path);
        }

        ht->FileMapping = CreateFileMappingW(ht->File, nullptr, PAGE_READWRITE, 0, 0, nullptr);
        if (ht->FileMapping == NULL)
        {
            return OpenError(ht, "Не удалось создать образ файла.", ::GetLastError(), path);
        }
        ht->Addr = MapViewOfFile(ht->FileMapping, FILE_MAP_WRITE, 0, 0, static_cast<SIZE_T>(size));
        if (ht->Addr == nullptr)
        {
            return OpenError(ht, "Не удалось отобразить файл в память.", ::GetLastError(), path);
        }

        ZeroMemory(ht->Addr, static_cast<SIZE_T>(size));
        const StorageHeader header = {
            FILE_MAGIC, FILE_VERSION, capacity, snapshot_interval, max_key_length, max_payload_length, 0
        };
        memcpy(ht->Addr, &header, sizeof(header));
        ReadParameters(ht);
        ClearError(ht);
        return ht;
    }


    HTHANDLE* Start(const char* file_name)
    {
        return OpenStorage(file_name, true);
    }


    HTHANDLE* Open(const char* file_name)
    {
        return OpenStorage(file_name, false);
    }


    BOOL Snap(const HTHANDLE* ht)
    {
        if (ht == nullptr || ht->Addr == nullptr)
        {
            return SetError(ht, "Хранилище не открыто.", ERROR_INVALID_HANDLE);
        }

        MutexLock lock(ht->Mutex);
        if (lock.Error != ERROR_SUCCESS)
        {
            return SetError(ht, "Не удалось заблокировать хранилище.", lock.Error);
        }
        if (!CheckRunning(ht)) return FALSE;

        return SaveStorage(ht);
    }

    
    BOOL Close(const HTHANDLE* ht)
    {
        if (ht == nullptr)
        {
            return SetError(nullptr, "Хранилище не открыто.", ERROR_INVALID_HANDLE);
        }

        BOOL saved = TRUE;
        if (ht->ReadyEvent != NULL || ht->Mutex == NULL)
        {
            MutexLock lock(ht->Mutex);
            if (lock.Error != ERROR_SUCCESS)
            {
                saved = SetError(ht, "Не удалось заблокировать хранилище.", lock.Error);
            }
            else
            {
                if (ht->ReadyEvent != NULL)
                {
                    CloseHandle(ht->ReadyEvent);
                    const_cast<HTHANDLE*>(ht)->ReadyEvent = NULL;
                }
                saved = SaveStorage(ht);
            }
        }

        DWORD error = saved ? ERROR_SUCCESS : ::GetLastError();
        ReleaseStorage(ht);
        if (saved) ClearError(nullptr);
        else ::SetLastError(error);
        return saved;
    }

    
    BOOL Insert(const HTHANDLE* ht, const Element* element)
    {
        if (!ValidKey(ht, element) || !ValidPayload(ht, element->payload, element->payloadlength))
        {
            return FALSE;
        }

        MutexLock lock(ht->Mutex);
        if (lock.Error != ERROR_SUCCESS) return SetError(ht, "Не удалось заблокировать хранилище.", lock.Error);
        if (!CheckRunning(ht)) return FALSE;

        BYTE* free_slot;
        if (FindSlot(ht, element, free_slot) != nullptr)
        {
            return SetError(ht, "Ключ уже существует.", ERROR_ALREADY_EXISTS);
        }
        if (free_slot == nullptr)
        {
            return SetError(ht, "Хранилище заполнено.", ERROR_DISK_FULL);
        }

        memcpy(KeyAddress(free_slot), element->key, element->keylength);
        WritePayload(ht, free_slot, element->payload, element->payloadlength);
        SlotHeader header = { ReadSlot(free_slot).State, element->keylength, element->payloadlength };
        memcpy(free_slot, &header, sizeof(header));
        MemoryBarrier();
        free_slot[0] = SLOT_BUSY;
        ClearError(ht);
        return TRUE;
    }

    
    BOOL Delete(const HTHANDLE* ht, const Element* element)
    {
        if (!ValidKey(ht, element)) return FALSE;
        MutexLock lock(ht->Mutex);
        if (lock.Error != ERROR_SUCCESS) return SetError(ht, "Не удалось заблокировать хранилище.", lock.Error);
        if (!CheckRunning(ht)) return FALSE;

        BYTE* slot = FindExistingSlot(ht, element);
        if (slot == nullptr) return FALSE;

        slot[0] = SLOT_DELETED;
        ClearError(ht);
        return TRUE;
    }

    
    Element* Get(const HTHANDLE* ht, const Element* element)
    {
        if (!ValidKey(ht, element)) return nullptr;
        MutexLock lock(ht->Mutex);
        if (lock.Error != ERROR_SUCCESS)
        {
            SetError(ht, "Не удалось заблокировать хранилище.", lock.Error);
            return nullptr;
        }
        if (!CheckRunning(ht)) return nullptr;

        BYTE* slot = FindExistingSlot(ht, element);
        if (slot == nullptr) return nullptr;

        const SlotHeader info = ReadSlot(slot);
        if (info.PayloadLength < 0 || info.PayloadLength > ht->MaxPayloadLength)
        {
            SetError(ht, "Повреждён размер данных.", ERROR_INVALID_DATA);
            return nullptr;
        }

        size_t size = sizeof(Element) + static_cast<size_t>(info.KeyLength) + info.PayloadLength;
        BYTE* copy = static_cast<BYTE*>(HeapAlloc(GetProcessHeap(), 0, size));
        if (copy == nullptr)
        {
            SetError(ht, "Недостаточно памяти.", ERROR_NOT_ENOUGH_MEMORY);
            return nullptr;
        }

        BYTE* key = copy + sizeof(Element);
        BYTE* data = key + info.KeyLength;
        memcpy(key, KeyAddress(slot), info.KeyLength);
        memcpy(data, PayloadAddress(ht, slot), info.PayloadLength);
        ClearError(ht);
        return new (copy) Element(key, info.KeyLength, data, info.PayloadLength);
    }

    
    void FreeElement(Element* element)
    {
        if (element != nullptr)
        {
            element->~Element();
            HeapFree(GetProcessHeap(), 0, element);
        }
    }

    
    BOOL Update(const HTHANDLE* ht, const Element* oldelement, const void* new_payload, int new_payload_length)
    {
        if (!ValidKey(ht, oldelement) || !ValidPayload(ht, new_payload, new_payload_length))
        {
            return FALSE;
        }
        MutexLock lock(ht->Mutex);
        if (lock.Error != ERROR_SUCCESS) return SetError(ht, "Не удалось заблокировать хранилище.", lock.Error);
        if (!CheckRunning(ht)) return FALSE;

        BYTE* slot = FindExistingSlot(ht, oldelement);
        if (slot == nullptr) return FALSE;

        WritePayload(ht, slot, new_payload, new_payload_length);
        SlotHeader info = ReadSlot(slot);
        info.PayloadLength = new_payload_length;
        memcpy(slot, &info, sizeof(info));
        ClearError(ht);
        return TRUE;
    }

    
    BOOL Increment(const HTHANDLE* ht, const Element* element, int32_t* result)
    {
        if (!ValidKey(ht, element))
        {
            return FALSE;
        }

        MutexLock lock(ht->Mutex);
        if (lock.Error != ERROR_SUCCESS)
        {
            return SetError(ht, "Не удалось заблокировать хранилище.", lock.Error);
        }
        if (!CheckRunning(ht)) return FALSE;

        BYTE* slot = FindExistingSlot(ht, element);
        if (slot == nullptr) return FALSE;
        if (ReadSlot(slot).PayloadLength != sizeof(int32_t))
        {
            return SetError(ht, "Значение должно быть 32-битным числом.", ERROR_INVALID_DATA);
        }

        int32_t value;
        memcpy(&value, PayloadAddress(ht, slot), sizeof(value));
        if (value == INT32_MAX)
        {
            return SetError(ht, "Достигнуто максимальное значение числа.", ERROR_ARITHMETIC_OVERFLOW);
        }
        ++value;
        memcpy(PayloadAddress(ht, slot), &value, sizeof(value));
        if (result != nullptr) *result = value;
        ClearError(ht);
        return TRUE;
    }

    
    char* GetLastError(HTHANDLE* ht)
    {
        return ht == nullptr ? last_error : ht->LastErrorMessage;
    }

    
    void print(const Element* element)
    {
        if (element == nullptr) return;
        printf("Ключ: ");
        PrintBytes(element->key, element->keylength);
        printf("| Данные: ");
        PrintBytes(element->payload, element->payloadlength);
        printf("\n");
    }
}