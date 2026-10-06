#pragma once

#include <windows.h>

#include <iostream>
#include <stdexcept>
#include <string>
#include <vector>

inline std::wstring sibling_exe(const wchar_t* name) {
    wchar_t path[32768]{};
    const DWORD length = GetModuleFileNameW(nullptr, path, 32768);
    if (length == 0 || length >= 32768) {
        throw std::runtime_error("GetModuleFileNameW failed");
    }
    std::wstring result(path, length);
    const auto separator = result.find_last_of(L"\\/");
    if (separator == std::wstring::npos) {
        throw std::runtime_error("Executable directory not found");
    }
    result.erase(separator + 1);
    result += name;
    return result;
}

inline bool launch_child(const wchar_t* application, std::wstring* command,
                         const wchar_t* label, std::vector<HANDLE>& children) {
    STARTUPINFOW startup{};
    startup.cb = sizeof(startup);
    PROCESS_INFORMATION info{};
    wchar_t* writable_command = command ? command->data() : nullptr;

    if (!CreateProcessW(application, writable_command, nullptr, nullptr, FALSE, 0,
                        nullptr, nullptr, &startup, &info)) {
        std::wcerr << label << L": CreateProcessW failed, error="
                   << GetLastError() << L'\n';
        return false;
    }
    std::wcout << label << L": child PID=" << info.dwProcessId << std::endl;
    CloseHandle(info.hThread);
    children.push_back(info.hProcess);
    return true;
}

inline int wait_and_close(std::vector<HANDLE>& children) {
    int result = 0;
    for (HANDLE process : children) {
        const DWORD wait_result = WaitForSingleObject(process, INFINITE);
        DWORD exit_code = 0;
        if (wait_result != WAIT_OBJECT_0 ||
            !GetExitCodeProcess(process, &exit_code)) {
            std::wcerr << L"Waiting for child failed, error="
                       << GetLastError() << L'\n';
            result = 1;
        } else {
            std::wcout << L"Child exit code=" << exit_code << std::endl;
            if (exit_code != 0) result = 1;
        }
        CloseHandle(process);
    }
    children.clear();
    return result;
}
