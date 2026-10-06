#include <windows.h>

#include <iostream>
#include <string>

#include "win_launch.hpp"

int wmain() {
    const std::wstring child = sibling_exe(L"Lab-03-startup-child.exe");
    wchar_t message[] = L"Hello, my dear child!";
    STARTUPINFOW startup{};
    startup.cb = sizeof(startup);
    startup.lpTitle = message;
    PROCESS_INFORMATION info{};

    // lpTitle is defined for a console child that creates a new console.
    if (!CreateProcessW(child.c_str(), nullptr, nullptr, nullptr, FALSE,
                        CREATE_NEW_CONSOLE, nullptr, nullptr,
                        &startup, &info)) {
        std::wcerr << L"CreateProcessW failed, error=" << GetLastError() << L'\n';
        return 1;
    }
    CloseHandle(info.hThread);
    std::wcout << L"Parent PID=" << GetCurrentProcessId()
               << L" child PID=" << info.dwProcessId << L'\n';
    const DWORD waited = WaitForSingleObject(info.hProcess, INFINITE);
    DWORD exit_code = 1;
    if (waited != WAIT_OBJECT_0 ||
        !GetExitCodeProcess(info.hProcess, &exit_code)) {
        std::wcerr << L"Child wait failed, error=" << GetLastError() << L'\n';
        exit_code = 1;
    }
    CloseHandle(info.hProcess);
    return static_cast<int>(exit_code);
}
