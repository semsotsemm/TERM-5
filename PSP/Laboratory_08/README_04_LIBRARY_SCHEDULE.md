# Задание 4. Расписание загрузки библиотек по почте

Сначала выполни изменения из [задания 3](README_03_LIBRARIES.md) в отдельной копии. Администратор присылает полное расписание DLL по местному времени сервера. Запись действует с первой минуты до последней, исключая последнюю. Ручная таблица из задания 3 и таблица расписания обслуживают новые запросы, причём ручная проверяется первой.

1. Создай `check_mail.py` рядом с исполняемым файлом по шагу 1 [задания 1](README_01_SCHEDULE.md). Он принимает тему письма и имя файла в аргументах. Код проверки сервисов из задания 1 в эту копию переносить не нужно.

2. В `ConcurrentServer/ConcurrentServer.cpp` добавь `#include <algorithm>`, `#include <fstream>` и `#include <sstream>`. Сразу после `list<shared_ptr<ServiceLibrary>> libraries;` добавь `list<shared_ptr<ServiceLibrary>> scheduled_libraries;`. Замени всю функцию `DispatchService`.

```cpp
HANDLE DispatchService(Contact& contact)
{
    lock_guard<mutex> lock(libraries_mutex);
    for (const auto* collection : { &libraries, &scheduled_libraries })
    {
        for (const auto& lib : *collection)
        {
            HANDLE service = lib->dispatch(contact.service_name, &contact);
            if (service != NULL)
            {
                contact.library = lib;
                return service;
            }
            if (GetLastError() != ERROR_NOT_FOUND) return NULL;
        }
    }
    SetLastError(ERROR_NOT_FOUND);
    return NULL;
}
```

3. Перед `DWORD WINAPI DispatchServer` вставь следующие функции. Они принимают только имена DLL рядом с сервером. Чередование расписаний не прерывает уже запущенные сервисы.

```cpp
int ReadClock(const string& text)
{
    if (text.size() != 5 || text[2] != ':' ||
        text.find_first_not_of("0123456789", 0) != 2 ||
        text.find_first_not_of("0123456789", 3) != string::npos)
    {
        return -1;
    }
    int hour = stoi(text.substr(0, 2));
    int minute = stoi(text.substr(3));
    if (hour == 24 && minute == 0) return 1440;
    return hour < 24 && minute < 60 ? hour * 60 + minute : -1;
}

int MAIL_POLL_SECONDS = 10;

struct LibraryHours
{
    string path;
    int first_minute;
    int last_minute;
};

vector<LibraryHours> library_hours;

void ReadLibrarySchedule()
{
    static ULONGLONG next_check = 0;
    ULONGLONG now = GetTickCount64();
    if (now < next_check) return;
    next_check = now + MAIL_POLL_SECONDS * 1000ULL;

    ifstream file("libraries.schedule");
    if (file)
    {
        vector<LibraryHours> received;
        bool valid = true;
        string line;
        while (getline(file, line))
        {
            istringstream row(line);
            string name, start, finish, extra;
            if (!(row >> name)) continue;
            if (!(row >> start >> finish) || row >> extra ||
                name.find_first_of("/\\:") != string::npos ||
                name.size() <= 4 || _stricmp(name.c_str() + name.size() - 4, ".dll") != 0)
            {
                valid = false;
                break;
            }
            int first = ReadClock(start);
            int last = ReadClock(finish);
            string path = LibraryPath(name);
            if (path.empty() || first < 0 || last <= first)
            {
                valid = false;
                break;
            }
            received.push_back({ path, first, last });
        }
        if (valid && !file.bad()) library_hours.swap(received);
    }

    SYSTEMTIME date;
    GetLocalTime(&date);
    int minute = date.wHour * 60 + date.wMinute;
    list<shared_ptr<ServiceLibrary>> current;
    lock_guard<mutex> lock(libraries_mutex);
    for (const auto& item : library_hours)
    {
        if (minute < item.first_minute || minute >= item.last_minute) continue;
        auto same = [&](const shared_ptr<ServiceLibrary>& lib)
        {
            return _stricmp(lib->name.c_str(), item.path.c_str()) == 0;
        };
        if (find_if(current.begin(), current.end(), same) != current.end()) continue;

        auto previous = find_if(scheduled_libraries.begin(), scheduled_libraries.end(), same);
        if (previous != scheduled_libraries.end())
        {
            current.push_back(*previous);
            continue;
        }

        auto lib = make_shared<ServiceLibrary>();
        lib->name = item.path;
        lib->module = LoadLibraryA(item.path.c_str());
        if (lib->module)
            lib->dispatch = (SSS_Function)GetProcAddress(lib->module, "SSS");
        if (lib->dispatch) current.push_back(lib);
        else LogMessage("[Расписание] Не удалось загрузить " + item.path + ".");
    }
    scheduled_libraries.swap(current);
}
```

4. В `DispatchServer` сразу за `WaitForSingleObjectEx(g_hAcceptEvent, 20, TRUE);` добавь `ReadLibrarySchedule();`. В `main` замени обе проверки `argc > 7` на `argc > 8`. После чтения таймаута прочитай седьмой параметр и добавь его в справку как `[Проверка почты, сек]`. В конце `main` рядом с `libraries.clear();` добавь `scheduled_libraries.clear();`.

```cpp
        if (argc > 7)
        {
            MAIL_POLL_SECONDS = ParseNumber(argv[7], 1, 3600);
        }
```

5. Собери Release вместе с `ExtraLibrary.dll` из задания 3. В `x64/Release` установи почтовые переменные как в задании 1. Каждую команду запусти в своём окне.

```powershell
py -3 .\check_mail.py 'LAB08 LIBRARIES' libraries.schedule 5
.\ConcurrentServer.exe 2000 ServiceLibrary.dll '\\.\pipe\ServerPipe' 2000 FindServer 180 5
```

6. Сохрани `ExtraLibrary.dll 00:00 24:00` в `libraries.txt`. В окне, где заданы параметры SMTP и ящика из задания 1, отправь письмо.

```powershell
py -3 .\check_mail.py send 'LAB08 LIBRARIES' libraries.txt
```

После двух интервалов опроса запроси `extra`. Для проверки выгрузки пришли новое расписание с интервалом вне текущего времени. Открытая сессия продолжит работу, новый запрос `extra` получит отказ, а `echo` из базовой DLL останется доступен.
