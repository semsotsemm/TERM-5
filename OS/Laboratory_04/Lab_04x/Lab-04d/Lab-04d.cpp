#include <windows.h>
#include <tlhelp32.h>
#include <stdio.h>
#include <stdlib.h>
#include <locale.h>

int main() {
    setlocale(LC_ALL, "Russian");

    DWORD currentPid = GetCurrentProcessId();
    wprintf(L"PID: %lu\n", currentPid);

    HANDLE hThreadSnap = INVALID_HANDLE_VALUE;
    THREADENTRY32 te32;

    hThreadSnap = CreateToolhelp32Snapshot(TH32CS_SNAPTHREAD, 0);
    if (hThreadSnap == INVALID_HANDLE_VALUE) {
        return 1;
    }

    te32.dwSize = sizeof(THREADENTRY32);

    if (!Thread32First(hThreadSnap, &te32)) {
        CloseHandle(hThreadSnap);
        return 1;
    }

    do {
        if (te32.th32OwnerProcessID == currentPid) {
            wprintf(L"TID: %lu\n", te32.th32ThreadID);
        }
    } while (Thread32Next(hThreadSnap, &te32));

    CloseHandle(hThreadSnap);

    system("pause");
    return 0;
}