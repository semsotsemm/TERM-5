# Задание 3. Загрузка и выгрузка DLL

Сервер ищет сервис в загруженных DLL в порядке их загрузки. При `UNLOAD_LIB` библиотека сразу убирается из таблицы для новых запросов. Уже запущенные сервисы и их APC сохраняют ссылку на DLL до уборки `Contact`.

1. В `ServerCommon.h` добавь `#include <memory>`. Перед `struct Contact` вставь `struct ServiceLibrary;`. В конец `Contact` добавь `std::shared_ptr<ServiceLibrary> library;`.

2. В `ConcurrentServer/ConcurrentServer.cpp` удали старые строки `typedef HANDLE(*SSS_Function)(const char*, LPVOID);` и `SSS_Function SSS = NULL;`. На их месте вставь этот код.

```cpp
typedef HANDLE(*SSS_Function)(const char*, LPVOID);

struct ServiceLibrary
{
    HMODULE module = NULL;
    SSS_Function dispatch = nullptr;
    string name;

    ~ServiceLibrary()
    {
        if (module) FreeLibrary(module);
    }
};

mutex libraries_mutex;
list<shared_ptr<ServiceLibrary>> libraries;

string LibraryPath(const string& name)
{
    char path[MAX_PATH];
    DWORD length = GetFullPathNameA(name.c_str(), MAX_PATH, path, NULL);
    return length > 0 && length < MAX_PATH ? string(path, length) : "";
}

bool LoadServiceLibrary(const string& name)
{
    string path = LibraryPath(name);
    if (path.empty()) return false;
    lock_guard<mutex> lock(libraries_mutex);
    for (const auto& lib : libraries)
        if (_stricmp(lib->name.c_str(), path.c_str()) == 0) return false;

    auto lib = make_shared<ServiceLibrary>();
    lib->name = path;
    lib->module = LoadLibraryA(path.c_str());
    if (!lib->module) return false;
    lib->dispatch = (SSS_Function)GetProcAddress(lib->module, "SSS");
    if (!lib->dispatch) return false;
    libraries.push_back(lib);
    return true;
}

bool UnloadServiceLibrary(const string& name)
{
    string path = LibraryPath(name);
    lock_guard<mutex> lock(libraries_mutex);
    for (auto it = libraries.begin(); it != libraries.end(); ++it)
    {
        if (_stricmp((*it)->name.c_str(), path.c_str()) == 0)
        {
            libraries.erase(it);
            return true;
        }
    }
    return false;
}

HANDLE DispatchService(Contact& contact)
{
    lock_guard<mutex> lock(libraries_mutex);
    for (const auto& lib : libraries)
    {
        HANDLE service = lib->dispatch(contact.service_name, &contact);
        if (service != NULL)
        {
            contact.library = lib;
            return service;
        }
        if (GetLastError() != ERROR_NOT_FOUND) return NULL;
    }
    SetLastError(ERROR_NOT_FOUND);
    return NULL;
}

string LibraryCommand(const string& input)
{
    size_t delimiter = input.find(' ');
    if (delimiter == string::npos) return "nocmd";
    string command = input.substr(0, delimiter);
    if (command != "LOAD_LIB" && command != "UNLOAD_LIB") return "nocmd";
    size_t first = input.find_first_not_of(' ', delimiter);
    if (first == string::npos) return "Нужно имя DLL.";
    size_t last = input.find_last_not_of(" \r\t");
    string name = input.substr(first, last - first + 1);
    if (name.size() >= 2 && name.front() == '"' && name.back() == '"')
        name = name.substr(1, name.size() - 2);
    if (command == "LOAD_LIB")
        return LoadServiceLibrary(name) ? "Библиотека загружена: " + name : "Не удалось загрузить: " + name;
    return UnloadServiceLibrary(name) ? "Библиотека убрана из списка: " + name : "Библиотека не найдена: " + name;
}
```

3. В `DispatchServer` замени `contact.h_service_thread = SSS(contact.service_name, &contact);` на `contact.h_service_thread = DispatchService(contact);`. В `main` удали участок от `HMODULE h_service_lib = LoadLibraryA...` до первого `LogMessage("========== ConcurrentServer...`, оставив сам `LogMessage`. На его месте вставь блок загрузки. Удаление `Contact` в `GarbageCleaner` уже происходит после завершения потока и APC.

```cpp
    if (!LoadServiceLibrary(DLL_NAME))
    {
        cerr << "Не удалось загрузить " << DLL_NAME << " или найти функцию SSS.\n";
        WSACleanup();
        return 1;
    }
```

4. В конце `main` замени `FreeLibrary(h_service_lib);` на `libraries.clear();`. К этому моменту все потоки уже завершены.

5. В разборе команд `ConsolePipe` замени `response = "nocmd";` на `response = LibraryCommand(input);`. В журнале этой ветки выводи `response` вместе с `input`. В справке `ClientNP/ClientNP.cpp` добавь `LOAD_LIB <имя.dll>` и `UNLOAD_LIB <имя.dll>`. Строка `Ответ сервера` уже выводит результат.

6. Для проверки подготовь DLL с новым сервисом. Скопируй проект `ServiceLibrary` рядом под именем `ExtraLibrary` и переименуй файл проекта в `ExtraLibrary.vcxproj`. В копии задай новый `ProjectGuid` через `[guid]::NewGuid().ToString('B')`, затем добавь проект в решение через Visual Studio. Сохрани экспорт `SSS` и общий `ServerCommon.h`. В копии замени весь массив `TableService` следующим.

```cpp
ServiceList TableService[] = {
    { "extra", TimeServer }
};
```

В копии `dllmain.cpp` замени `request == "time"` внутри `TimeServer` на `request == "extra"`. В `ClientT/ClientT.cpp` добавь `|| request_cmd == "extra"` в вычисление `known_service`. Клиент уже передаёт имя сервиса как запрос и отображает ответ `DATA`.

7. Пересобери **все** DLL и сервер из-за изменения `Contact`. Убедись, что `ExtraLibrary.dll` лежит рядом с `ConcurrentServer.exe`. В `ClientNP` выполни этот сценарий.

```text
LOAD_LIB ExtraLibrary.dll
UNLOAD_LIB ExtraLibrary.dll
UNLOAD_LIB ServiceLibrary.dll
LOAD_LIB ServiceLibrary.dll
```

Между первой и второй командами запусти долгий сеанс `extra`. После второй команды он доработает, а новый `extra` получит `ErrorInquiry`. После выгрузки `ServiceLibrary.dll` любой известный запрос получит отказ. Последняя команда вернёт `echo`, `time` и `rand`.
