#pragma once
#include <windows.h>
#include <clocale>
#include <iostream>
#include <string>

constexpr DWORD PIPE_BUFFER_SIZE = 512;
constexpr DWORD PIPE_WAIT_MS = 10000;

inline std::string Utf8(const std::wstring& text)
{
    if (text.empty()) return {};
    const int size = WideCharToMultiByte(CP_UTF8, 0, text.data(),
        static_cast<int>(text.size()), nullptr, 0, nullptr, nullptr);
    std::string result(size, '\0');
    WideCharToMultiByte(CP_UTF8, 0, text.data(), static_cast<int>(text.size()),
        &result[0], size, nullptr, nullptr);
    return result;
}

inline void InitConsole()
{
    std::setlocale(LC_ALL, ".UTF8");
    SetConsoleOutputCP(CP_UTF8);
    SetConsoleCP(CP_UTF8);
}

inline std::string SetErrorMessageText(const std::string& operation, DWORD code)
{
    LPWSTR systemText = nullptr;
    FormatMessageW(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM |
        FORMAT_MESSAGE_IGNORE_INSERTS, nullptr, code, 0,
        reinterpret_cast<LPWSTR>(&systemText), 0, nullptr);
    std::string result = operation + std::to_string(code) + ": ";
    if (systemText)
    {
        result += Utf8(systemText);
        LocalFree(systemText);
    }
    else result += "Неизвестная системная ошибка.\n";
    switch (code)
    {
    case ERROR_ACCESS_DENIED:
    case ERROR_LOGON_FAILURE:
    case ERROR_LOGON_TYPE_NOT_GRANTED:
    case ERROR_ACCOUNT_RESTRICTION:
    case ERROR_INVALID_PASSWORD:
    case ERROR_NOT_AUTHENTICATED:
        result += "Проверьте вход в \\\\СЕРВЕР\\IPC$ под учетной записью сервера "
            "с непустым паролем, а также права на канал. PIN Windows Hello не подходит.\n";
        break;
    case ERROR_BAD_NETPATH:
    case ERROR_BAD_NET_NAME:
    case ERROR_NETWORK_UNREACHABLE:
    case ERROR_SEM_TIMEOUT:
        result += "Проверьте адрес сервера, TCP 445, службу Сервер и брандмауэр.\n";
        break;
    case ERROR_FILE_NOT_FOUND:
    case ERROR_PATH_NOT_FOUND:
        result += "Сначала запустите ServerNP на указанном ПК. Имя канала: Tube.\n";
        break;
    case ERROR_SESSION_CREDENTIAL_CONFLICT:
        result += "Уже есть SMB-сеанс к этому серверу с другой учетной записью. "
            "Проверьте net use; не удаляйте чужие подключения вслепую.\n";
        break;
    case ERROR_PIPE_BUSY:
        result += "Канал занят другим клиентом; дождитесь его отключения.\n";
        break;
    }
    return result;
}

inline std::wstring GetPipeName(int argc, wchar_t* argv[])
{
    if (argc > 2)
        throw std::string("Использование: клиент.exe [имя_или_IP_сервера]; для своего ПК: .\n");
    // Раньше два клиента шли только на свой ПК, а один — на фиксированный DESKTOP.
    const std::wstring host = argc == 2 ? argv[1] : L".";
    if (host.empty() || host.find_first_of(L"\\/ \t") != std::wstring::npos)
        throw std::string("Укажите только имя или IPv4 сервера, без \\\\ и \\pipe\\Tube.\n");
    return L"\\\\" + host + L"\\pipe\\Tube";
}

inline HANDLE OpenPipe(const std::wstring& pipeName)
{
    const ULONGLONG deadline = GetTickCount64() + PIPE_WAIT_MS;
    while (true)
    {
        HANDLE pipe = CreateFileW(pipeName.c_str(), GENERIC_READ | GENERIC_WRITE,
            0, nullptr, OPEN_EXISTING,
            SECURITY_SQOS_PRESENT | SECURITY_IDENTIFICATION, nullptr);
        if (pipe != INVALID_HANDLE_VALUE) return pipe;
        const DWORD error = GetLastError();
        if (error != ERROR_PIPE_BUSY)
            throw SetErrorMessageText("Ошибка CreateFile: ", error);
        const ULONGLONG now = GetTickCount64();
        if (now >= deadline)
            throw SetErrorMessageText("Тайм-аут ожидания канала: ", ERROR_SEM_TIMEOUT);
        if (!WaitNamedPipeW(pipeName.c_str(), static_cast<DWORD>(deadline - now)))
            throw SetErrorMessageText("Ошибка WaitNamedPipe: ", GetLastError());
        // WaitNamedPipe не резервирует экземпляр: другой клиент мог занять его раньше.
    }
}

inline void SetMessageMode(HANDLE pipe)
{
    DWORD mode = PIPE_READMODE_MESSAGE;
    if (!SetNamedPipeHandleState(pipe, &mode, nullptr, nullptr))
        throw SetErrorMessageText("Ошибка SetNamedPipeHandleState: ", GetLastError());
}

inline bool ReadPipeMessage(HANDLE pipe, std::string& message)
{
    message.clear();
    while (true)
    {
        char buffer[PIPE_BUFFER_SIZE];
        DWORD bytesRead = 0;
        const BOOL ok = ReadFile(pipe, buffer, sizeof(buffer), &bytesRead, nullptr);
        const DWORD error = ok ? ERROR_SUCCESS : GetLastError();
        if (!ok && error != ERROR_MORE_DATA)
        {
            if (error == ERROR_BROKEN_PIPE || error == ERROR_NO_DATA ||
                error == ERROR_PIPE_NOT_CONNECTED) return false;
            throw SetErrorMessageText("Ошибка ReadFile: ", error);
        }
        // buffer[bytesRead] = '\0' при 512 байтах выходило за границу массива.
        message.append(buffer, bytesRead);
        if (message.size() > 64 * 1024)
            throw std::string("Сообщение превышает ограничение 64 КиБ.\n");
        if (ok)
        {
            if (!message.empty() && message.back() == '\0') message.pop_back();
            return true;
        }
    }
}
