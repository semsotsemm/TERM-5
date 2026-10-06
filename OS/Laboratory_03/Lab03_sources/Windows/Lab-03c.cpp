#include <windows.h>
#include <tlhelp32.h>

#include <iostream>

int wmain() {
    const HANDLE snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (snapshot == INVALID_HANDLE_VALUE) {
        std::wcerr << L"CreateToolhelp32Snapshot failed, error="
                   << GetLastError() << L'\n';
        return 1;
    }

    PROCESSENTRY32W entry{};
    entry.dwSize = sizeof(entry);
    if (!Process32FirstW(snapshot, &entry)) {
        std::wcerr << L"Process32FirstW failed, error=" << GetLastError() << L'\n';
        CloseHandle(snapshot);
        return 1;
    }

    do {
        std::wcout << entry.szExeFile
                   << L" PID=" << entry.th32ProcessID
                   << L" PPID=" << entry.th32ParentProcessID << L'\n';
    } while (Process32NextW(snapshot, &entry));

    const DWORD final_error = GetLastError();
    CloseHandle(snapshot);
    if (final_error != ERROR_NO_MORE_FILES) {
        std::wcerr << L"Process32NextW failed, error=" << final_error << L'\n';
        return 1;
    }
    return 0;
}
