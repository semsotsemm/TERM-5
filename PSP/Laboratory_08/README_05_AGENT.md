# Задание 5. Агент наблюдения за сервером

Здесь агент раз в несколько секунд читает статистику и сообщает, если сервер недоступен. У агента свой named pipe, поэтому `ClientNP` остаётся доступен для команд.

1. В `ConcurrentServer/ConcurrentServer.cpp` перед `DWORD WINAPI ConsolePipe` добавь поток канала агента. При нестандартном `PIPE_NAME` он получит суффикс `Agent`.

```cpp
DWORD WINAPI AgentPipe(LPVOID lp_param)
{
    auto* command = (atomic<TalkersCommand>*)lp_param;
    string name = PIPE_NAME + "Agent";
    HANDLE pipe = CreateNamedPipeA(name.c_str(), PIPE_ACCESS_OUTBOUND,
        PIPE_TYPE_BYTE | PIPE_READMODE_BYTE | PIPE_NOWAIT, 1, 4096, 0, 0, NULL);
    if (pipe == INVALID_HANDLE_VALUE)
    {
        LogMessage("[AgentPipe] Не удалось создать канал: " + to_string(GetLastError()));
        return 1;
    }

    while (command->load() != EXIT)
    {
        if (!ConnectNamedPipe(pipe, NULL))
        {
            DWORD error = GetLastError();
            if (error != ERROR_PIPE_CONNECTED)
            {
                if (error == ERROR_NO_DATA) DisconnectNamedPipe(pipe);
                Sleep(20);
                continue;
            }
        }

        string reply = GetStatistics();
        DWORD written = 0;
        if (WriteFile(pipe, reply.data(), static_cast<DWORD>(reply.size()), &written, NULL) &&
            written == reply.size())
        {
            // Клиент сначала читает сообщение и затем закрывает свой конец.
            ULONGLONG deadline = GetTickCount64() + 1000;
            while (command->load() != EXIT && GetTickCount64() < deadline)
            {
                Sleep(20);
            }
        }
        DisconnectNamedPipe(pipe);
    }

    CloseHandle(pipe);
    return 0;
}
```

2. В `main` замени `HANDLE threads[5]` на `HANDLE threads[6]`. После создания `threads[4]` вставь следующий вызов. Добавь `&& threads[5]` к проверке запуска, шестой `THREAD_PRIORITY_NORMAL` в массив `priorities` и замени границу `i < 5` на `i < 6` в обоих циклах по этим потокам.

```cpp
        threads[5] = CreateThread(NULL, 0, AgentPipe, &cmd_start, CREATE_SUSPENDED, NULL);
```

3. В `x64/Release` создай `ServerAgent.ps1`.

```powershell
param(
    [string]$Server = '.',
    [string]$PipeName = 'ServerPipeAgent',
    [ValidateRange(1, 3600)][int]$Interval = 5
)

while ($true) {
    $pipe = [IO.Pipes.NamedPipeClientStream]::new($Server, $PipeName, [IO.Pipes.PipeDirection]::In)
    try {
        $pipe.Connect(1000)
        $reader = [IO.StreamReader]::new($pipe, [Text.Encoding]::UTF8)
        try {
            $text = $reader.ReadToEnd()
            if (-not $text.StartsWith("statistics" + [char]10)) { throw 'Неверный ответ агента' }
            Write-Output ("[" + (Get-Date -Format 'HH:mm:ss') + "] " + $Server)
            Write-Output $text.Substring(11)
        } finally {
            $reader.Dispose()
        }
    } catch {
        Write-Warning ("Сервер {0} недоступен: {1}" -f $Server, $_.Exception.Message)
    } finally {
        $pipe.Dispose()
    }
    Start-Sleep -Seconds $Interval
}
```

4. Собери Release и запусти `ConcurrentServer.exe`. Открой `ClientNP.exe . ServerPipe` в одном окне, а агент в другом.

```powershell
powershell -NoProfile -File .\ServerAgent.ps1 -Server . -Interval 5
```

Подключи `ClientT`, проверь изменение числа активных клиентов в выводе агента и одновременно дай `statistics` в `ClientNP`. После `shutdown` агент сообщит о недоступности. Для остановки агента нажми `Ctrl+C`. При запуске с другого ПК укажи имя сервера в `-Server` и используй учётную запись с доступом к SMB.
