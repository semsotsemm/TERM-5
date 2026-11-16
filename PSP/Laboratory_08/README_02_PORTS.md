# Задание 2. Приём клиентов на нескольких портах

Сервер хранит открытые TCP-порты в одной таблице. Основной порт создаётся при старте. Открытие и закрытие порта применяются к новым подключениям, а обслуживаемые сокеты продолжают работу.

1. В `ConcurrentServer/ConcurrentServer.cpp` добавь `#include <map>` и `#include <sstream>`. Перед `DWORD WINAPI ResponseServer` вставь код таблицы портов.

```cpp
mutex listeners_mutex;
map<int, SOCKET> listeners;

bool OpenAcceptPort(int port)
{
    lock_guard<mutex> lock(listeners_mutex);
    if (listeners.count(port) != 0) return false;
    SOCKET listener = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (listener == INVALID_SOCKET) return false;
    SOCKADDR_IN address = {};
    address.sin_family = AF_INET;
    address.sin_port = htons(static_cast<u_short>(port));
    address.sin_addr.s_addr = INADDR_ANY;
    u_long nonblocking = 1;
    BOOL exclusive = TRUE;
    if (setsockopt(listener, SOL_SOCKET, SO_EXCLUSIVEADDRUSE, (const char*)&exclusive, sizeof(exclusive)) == SOCKET_ERROR ||
        bind(listener, (sockaddr*)&address, sizeof(address)) == SOCKET_ERROR ||
        listen(listener, SOMAXCONN) == SOCKET_ERROR ||
        ioctlsocket(listener, FIONBIO, &nonblocking) == SOCKET_ERROR)
    {
        closesocket(listener);
        return false;
    }
    listeners.emplace(port, listener);
    return true;
}

bool CloseAcceptPort(int port)
{
    lock_guard<mutex> lock(listeners_mutex);
    auto it = listeners.find(port);
    if (it == listeners.end()) return false;
    if (closesocket(it->second) == SOCKET_ERROR) return false;
    listeners.erase(it);
    return true;
}

int DiscoveryPort()
{
    lock_guard<mutex> lock(listeners_mutex);
    if (listeners.count(PORT) != 0) return PORT;
    return listeners.empty() ? 0 : listeners.begin()->first;
}

void CloseAcceptPorts()
{
    lock_guard<mutex> lock(listeners_mutex);
    for (const auto& listener : listeners)
        closesocket(listener.second);
    listeners.clear();
}

string AcceptPortCommand(const string& input)
{
    istringstream command(input);
    string action, number, extra;
    if (!(command >> action) || (action != "OPEN_ACCEPT" && action != "CLOSE_ACCEPT"))
        return "nocmd";
    if (!(command >> number) || command >> extra) return "Нужен один номер порта.";
    size_t end = 0;
    int port;
    try
    {
        port = stoi(number, &end);
    }
    catch (const exception&)
    {
        return "Некорректный номер порта.";
    }
    if (end != number.size() || port < 1 || port > 65535) return "Некорректный номер порта.";

    if (action == "OPEN_ACCEPT")
        return OpenAcceptPort(port) ? "Порт " + to_string(port) + " открыт." :
            "Не удалось открыть порт " + to_string(port) + ".";
    return CloseAcceptPort(port) ? "Порт " + to_string(port) + " закрыт." :
        "Не удалось закрыть порт " + to_string(port) + ".";
}
```

2. В `ResponseServer` перед строкой `string invite = ...` вставь этот блок. В самой строке замени `to_string(PORT)` на `to_string(port)`.

```cpp
                int port = DiscoveryPort();
                if (port == 0) continue;
```

3. Удали параметр `SOCKET server_socket` из сигнатуры `CommandsCycle`. Замени строку `AcceptCycle(1, server_socket);` внутри `if (is_accepting && cmd.load() == GETCOMMAND)` следующим кодом.

```cpp
            lock_guard<mutex> lock(listeners_mutex);
            for (const auto& listener : listeners)
                AcceptCycle(1, listener.second);
```

4. В том же файле удали старую функцию `AcceptServer` целиком и вставь эту.

```cpp
DWORD WINAPI AcceptServer(LPVOID lp_param)
{
    auto* command = (atomic<TalkersCommand>*)lp_param;
    g_hAcceptEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
    if (g_hAcceptEvent == NULL || !OpenAcceptPort(PORT))
    {
        LogMessage("[AcceptServer] Не удалось запустить приём.");
        command->store(EXIT);
        SetEvent(g_hServerReady);
        return 1;
    }

    LogMessage("[AcceptServer] TCP-порт " + to_string(PORT) + ". Ожидание клиентов.");
    SetEvent(g_hServerReady);
    WaitForSingleObject(g_hWorkersReady, INFINITE);
    CommandsCycle(*command);
    CloseAcceptPorts();
    return 0;
}
```

5. В `ConsolePipe` замени `response = "nocmd";` внутри ветки разбора неизвестной команды на `response = AcceptPortCommand(input);`. После ветки журналирования `statistics` измени сообщение с `Неизвестная команда` на `Команда`, а литерал `Ответ: nocmd.` замени выводом `response`. В `ClientNP/ClientNP.cpp` допиши к `help` строки `OPEN_ACCEPT <порт>` и `CLOSE_ACCEPT <порт>`. Ветка `Ответ сервера` уже показывает результат.

6. Собери Release и открой `ClientNP.exe . ServerPipe`. Введи `OPEN_ACCEPT 2001`, затем `OPEN_ACCEPT 2002`. Проверь одновременно исходный порт и оба новых.

```powershell
.\ClientT.exe localhost 2000 1000
.\ClientT.exe localhost 2001 1000
.\ClientT.exe localhost 2002 1000
```

Введи `CLOSE_ACCEPT 2001`. Текущий клиент на 2001 продолжит обмен. Новый `ClientT.exe localhost 2001` получит отказ TCP. Затем снова выполни `OPEN_ACCEPT 2001` и подключись. `stop`, `start` и `wait` управляют приёмом сразу на всех открытых портах.
