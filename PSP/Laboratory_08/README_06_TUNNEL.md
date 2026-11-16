# Задание 6. Туннель между двумя серверами

Клиент открывает сервис `tunnel` на первом сервере. Тот открывает отдельный сеанс `echo` на втором, пересылает туда каждый запрос и передаёт клиенту полученный ответ. При остановке одного из серверов или истечении его таймера туннель закрывает оба сокета. Сервисы обоих серверов можно собирать из одной DLL.

1. В `ServiceLibrary/dllmain.cpp` добавь `#include <cstdlib>` и объявление `DWORD WINAPI TunnelServer(LPVOID lp_param);` рядом с объявлением `EchoServer`. В `TableService` добавь `{ "tunnel", TunnelServer }` последней строкой, поставив запятую после предыдущей строки.

2. Добавь третий параметр к `ReceiveServiceLine`. В начале этой функции выбери сокет клиента, когда третий аргумент не передан. Затем в этой функции замени аргумент `contact->client_socket` в вызовах `WaitForSocket` и `recv` на `socket`. Другие сервисы продолжают вызывать функцию с двумя аргументами.

```cpp
bool ReceiveServiceLine(Contact* contact, string& text, SOCKET socket = INVALID_SOCKET)
{
    if (socket == INVALID_SOCKET) socket = contact->client_socket;
    // Дальше идёт существующий код чтения строки.
```

3. Вставь перед экспортируемой функцией `SSS` следующий код.

```cpp
bool ConnectTunnel(Contact* contact, SOCKET peer, const SOCKADDR_IN& address)
{
    u_long mode = 1;
    if (ioctlsocket(peer, FIONBIO, &mode) == SOCKET_ERROR) return false;
    if (connect(peer, (const sockaddr*)&address, sizeof(address)) == SOCKET_ERROR)
    {
        if (WSAGetLastError() != WSAEWOULDBLOCK) return false;
        ULONGLONG deadline = GetTickCount64() + 3000;
        while (true)
        {
            if (ReadCounter(contact->finish_reason) != DONE || GetTickCount64() >= deadline)
                return false;
            fd_set write_set, error_set;
            FD_ZERO(&write_set);
            FD_ZERO(&error_set);
            FD_SET(peer, &write_set);
            FD_SET(peer, &error_set);
            timeval wait = { 0, 100000 };
            int ready = select(0, NULL, &write_set, &error_set, &wait);
            if (ready == 0) continue;
            if (ready == SOCKET_ERROR || FD_ISSET(peer, &error_set)) return false;
            break;
        }
    }
    mode = 0;
    DWORD timeout = 1000;
    return ioctlsocket(peer, FIONBIO, &mode) == 0 &&
        setsockopt(peer, SOL_SOCKET, SO_SNDTIMEO, (const char*)&timeout, sizeof(timeout)) == 0 &&
        EnableKeepAlive(peer);
}

bool ExchangeThroughTunnel(Contact* contact, SOCKET peer)
{
    string reply;
    if (!ReceiveServiceLine(contact, reply, peer) || reply.rfind("MAIL CONNECTED ", 0) != 0 ||
        !SendLine(peer, "echo") || !ReceiveServiceLine(contact, reply, peer) || reply.rfind("OK ", 0) != 0)
    {
        return false;
    }

    string request;
    while (ReceiveServiceLine(contact, request))
    {
        if (!SendLine(peer, request) || !ReceiveServiceLine(contact, reply, peer))
            return false;
        if (request == "stop")
            return reply == "MAIL DISCONNECTED DONE";
        if (reply.rfind("DATA ", 0) != 0 || !SendLine(contact->client_socket, reply))
            return false;
    }
    return false;
}

DWORD WINAPI TunnelServer(LPVOID lp_param)
{
    Contact* contact = (Contact*)lp_param;
    StartService(contact);
    char host[256] = {};
    char port_text[16] = {};
    DWORD host_length = GetEnvironmentVariableA("TUNNEL_HOST", host, sizeof(host));
    DWORD port_length = GetEnvironmentVariableA("TUNNEL_PORT", port_text, sizeof(port_text));
    char* end = port_text;
    long port = port_length == 0 ? 2001 : strtol(port_text, &end, 10);
    SOCKADDR_IN address = {};
    bool valid = host_length < sizeof(host) && port_length < sizeof(port_text) &&
        (port_length == 0 || (end != port_text && *end == '\0')) && port > 0 && port <= 65535;
    bool success = false;
    if (valid && ResolveServer(host_length == 0 ? "127.0.0.1" : host, static_cast<int>(port), address) == 0)
    {
        SOCKET peer = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
        if (peer != INVALID_SOCKET)
        {
            success = ConnectTunnel(contact, peer, address) && ExchangeThroughTunnel(contact, peer);
            closesocket(peer);
        }
    }
    if (!success && ReadCounter(contact->finish_reason) == DONE)
        contact->log_message("[TunnelServer] Обмен со вторым сервером не завершён.");
    FinishService(contact, !success);
    return 0;
}
```

4. В `ClientT/ClientT.cpp` допиши `|| request_cmd == "tunnel"` в условие `known_service`. В цикле отправки измени формирование запроса. В справку сервисов добавь `tunnel`.

```cpp
                string message = request_cmd == "echo" || request_cmd == "tunnel"
                    ? "Hello from Client " + to_string(i) : request_cmd;
```

5. Собери Release. Запусти второй сервер на порту 2001 в отдельном окне.

```powershell
.\ConcurrentServer.exe 2001 ServiceLibrary.dll '\\.\pipe\BackendPipe' 2001 FindBackend 180
```

6. На первом сервере задай IPv4 или DNS-имя второго сервера. Для проверки на одном ПК подойдут эти команды.

```powershell
$env:TUNNEL_HOST = '127.0.0.1'
$env:TUNNEL_PORT = '2001'
.\ConcurrentServer.exe 2000 ServiceLibrary.dll '\\.\pipe\ServerPipe' 2000 FindServer 180
```

Запусти `ClientT.exe localhost 2000 1000`, введи `tunnel` и `10`. На первом сервере в журнале должен быть `tunnel`, на втором должен появиться `echo` и номер TCP-порта клиента первого сервера. После завершения клиент покажет 10 ответов. Затем снова запусти долгий `tunnel`, останови второй сервер и проверь, что клиент получил ошибку соединения.
