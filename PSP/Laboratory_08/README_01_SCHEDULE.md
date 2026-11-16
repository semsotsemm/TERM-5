# Задание 1. Расписание сервисов по почте

Применяй изменения к отдельной копии базовой лабораторной. Письмо содержит полное расписание на каждый день по местному времени сервера. До первого письма доступны все сервисы. После первого письма сервис без интервала закрыт для новых запросов, а уже запущенные сеансы дорабатывают.

1. В каталоге запуска сервера создай `check_mail.py`. Он берёт текст письма из IMAP через TLS и обновляет файл, который читает сервер. Письма с другой темой или токеном остаются непрочитанными.

```python
import email
import imaplib
import os
import smtplib
import ssl
import sys
import time
from email import policy
from email.message import EmailMessage
from pathlib import Path

if sys.argv[1] == "send":
    message = EmailMessage()
    message["From"] = os.environ["LAB08_MAIL_USER"]
    message["To"] = os.environ["LAB08_MAIL_USER"]
    message["Subject"] = sys.argv[2]
    message["X-Lab08-Token"] = os.environ["LAB08_MAIL_TOKEN"]
    message.set_content(Path(sys.argv[3]).read_text(encoding="utf-8-sig"))
    with smtplib.SMTP_SSL(os.environ["LAB08_SMTP"], 465, timeout=10,
                          context=ssl.create_default_context()) as smtp:
        smtp.login(os.environ["LAB08_MAIL_USER"], os.environ["LAB08_MAIL_PASSWORD"])
        smtp.send_message(message)
    sys.exit(0)

subject = sys.argv[1]
target = Path(sys.argv[2]).resolve()
interval = int(sys.argv[3])
if interval < 1:
    raise ValueError("Интервал должен быть больше нуля")

while True:
    try:
        with imaplib.IMAP4_SSL(os.environ["LAB08_IMAP"], timeout=10,
                              ssl_context=ssl.create_default_context()) as box:
            box.login(os.environ["LAB08_MAIL_USER"], os.environ["LAB08_MAIL_PASSWORD"])
            if box.select("INBOX")[0] != "OK":
                raise RuntimeError("Не удалось открыть INBOX")
            status, ids = box.uid("SEARCH", None, "UNSEEN")
            if status != "OK":
                raise RuntimeError("Не удалось прочитать список писем")
            for uid in ids[0].split():
                status, parts = box.uid("FETCH", uid, "(BODY.PEEK[])")
                raw = next((part[1] for part in parts if isinstance(part, tuple)), None)
                if status != "OK" or raw is None:
                    continue
                message = email.message_from_bytes(raw, policy=policy.default)
                if message["Subject"] != subject or message["X-Lab08-Token"] != os.environ["LAB08_MAIL_TOKEN"]:
                    continue
                body = message.get_body(preferencelist=("plain",))
                if body is None:
                    continue
                temp = target.with_suffix(".tmp")
                temp.write_text(body.get_content(), encoding="utf-8")
                os.replace(temp, target)
                box.uid("STORE", uid, "+FLAGS", r"(\Seen)")
    except (OSError, imaplib.IMAP4.error, RuntimeError) as error:
        print(error, file=sys.stderr, flush=True)
    time.sleep(interval)
```

2. В `ConcurrentServer/ConcurrentServer.cpp` добавь `#include <fstream>` и `#include <sstream>`. Перед `DWORD WINAPI DispatchServer` вставь следующий код.

```cpp
int MAIL_POLL_SECONDS = 10;

struct ServiceHours
{
    string name;
    int first_minute;
    int last_minute;
};

vector<ServiceHours> service_hours;
bool has_service_schedule = false;

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

void ReadServiceSchedule()
{
    static ULONGLONG next_check = 0;
    ULONGLONG now = GetTickCount64();
    if (now < next_check) return;
    next_check = now + MAIL_POLL_SECONDS * 1000ULL;

    ifstream file("services.schedule");
    if (!file) return;
    vector<ServiceHours> received;
    string line;
    while (getline(file, line))
    {
        istringstream row(line);
        string name, start, finish, extra;
        if (!(row >> name)) continue;
        if (!(row >> start >> finish) || row >> extra) return;
        int first = ReadClock(start);
        int last = ReadClock(finish);
        if ((name != "echo" && name != "time" && name != "rand") || first < 0 || last <= first) return;
        received.push_back({ name, first, last });
    }
    if (file.bad()) return;
    service_hours.swap(received);
    has_service_schedule = true;
}

bool ServiceIsOpen(const string& name)
{
    if (!has_service_schedule) return true;
    SYSTEMTIME time;
    GetLocalTime(&time);
    int minute = time.wHour * 60 + time.wMinute;
    for (const auto& hours : service_hours)
    {
        if (hours.name == name && minute >= hours.first_minute && minute < hours.last_minute)
            return true;
    }
    return false;
}
```

3. В `DispatchServer` после вызова `WaitForSingleObjectEx(g_hAcceptEvent, 20, TRUE)` добавь `ReadServiceSchedule();`. После проверки `if (!request_ready)`, перед журналированием запроса, вставь проверку расписания.

```cpp
            if (!ServiceIsOpen(contact.service_name))
            {
                SendLine(contact.client_socket, "ErrorSchedule");
                FinishWithoutService(contact, "сервис закрыт по расписанию.");
                continue;
            }
```

4. В `ClientT/ClientT.cpp`, перед `if (answer == "ErrorInquiry")`, вставь этот блок. Существующий `if (answer == "ErrorInquiry")` замени на `else if (answer == "ErrorInquiry")`.

```cpp
        if (answer == "ErrorSchedule")
        {
            cout << "Сервис закрыт по расписанию.\n";
            WaitForReply(client_socket, pending, status, deadline, "Ожидаем письма об отключении.");
        }
```

5. В серверном `main` замени обе проверки `argc > 7` на `argc > 8`. После чтения `SERVICE_TIMEOUT` вставь следующий блок и добавь в справку аргумент `[Проверка почты, сек]`.

```cpp
        if (argc > 7)
        {
            MAIL_POLL_SECONDS = ParseNumber(argv[7], 1, 3600);
        }
```

6. Собери Release. В `x64/Release` открой два окна PowerShell. В первом установи параметры почтового ящика и запусти чтение. Подставь IMAP-хост сервиса, адрес ящика, пароль приложения и свой секретный токен.

```powershell
$env:LAB08_IMAP = 'imap.example.com'
$env:LAB08_MAIL_USER = 'lab08@example.com'
$credential = Get-Credential -UserName $env:LAB08_MAIL_USER -Message 'Пароль приложения для IMAP'
$env:LAB08_MAIL_PASSWORD = $credential.GetNetworkCredential().Password
$env:LAB08_MAIL_TOKEN = Read-Host 'Токен из заголовка X-Lab08-Token'
py -3 .\check_mail.py 'LAB08 SERVICES' services.schedule 5
```

```powershell
.\ConcurrentServer.exe 2000 ServiceLibrary.dll '\\.\pipe\ServerPipe' 2000 FindServer 180 5
```

7. Сохрани тело письма в `services.txt`. В третьем окне задай те же почтовые переменные и SMTP-хост ящика, затем отправь письмо с темой `LAB08 SERVICES` через тот же скрипт.

```text
echo 00:00 24:00
rand 08:00 12:00
time 09:00 10:00
```

```powershell
$env:LAB08_SMTP = 'smtp.example.com'
py -3 .\check_mail.py send 'LAB08 SERVICES' services.txt
```

Через два интервала опроса проверь `echo` и `time`. `echo` будет доступен сразу. Для проверки `time` отправь новое письмо с интервалом, в который попадает текущее время, и повтори запрос.
