#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <locale.h>

void Lab_04x(int iterations) {
    if (iterations <= 0) return;

    size_t user_size = 64;
    size_t name_size = 128;

    wchar_t* username = (wchar_t*)malloc(user_size * sizeof(wchar_t));
    wchar_t* fullname = (wchar_t*)malloc(name_size * sizeof(wchar_t));

    if (username == NULL || fullname == NULL) {
        if (username) free(username);
        if (fullname) free(fullname);
        return;
    }

    wcscpy_s(username, user_size, L"User-3af11550");
    wcscpy_s(fullname, name_size, L"Антипов Алексей Романович");

    size_t user_len = wcslen(username);
    size_t name_len = wcslen(fullname);

    DWORD pid = GetCurrentProcessId();
    DWORD tid = GetCurrentThreadId();

    srand((unsigned int)time(NULL) ^ tid);

    for (int i = 0; i < iterations; ++i) {
        wchar_t u_char = username[i % user_len];
        wchar_t f_char = fullname[i % name_len];

        wprintf(L"%lu - %lu - %d - %lc - %lc\n", pid, tid, i + 1, u_char, f_char);

        int delay = 350 + (rand() % 101);
        Sleep(delay);
    }

    free(username);
    free(fullname);
}

DWORD WINAPI ThreadWrapper(LPVOID lpParam) {
    int iterations = *((int*)lpParam);
    Lab_04x(iterations);
    return 0;
}

int main() {
    setlocale(LC_ALL, "Russian");

    DWORD processId = GetCurrentProcessId();
    wprintf(L"PID: %lu\n", processId);

    int iters_t1 = 55;
    int iters_t2 = 130;

    HANDLE hThreads[2];
    DWORD dwThreadId1, dwThreadId2;

    hThreads[0] = CreateThread(NULL, 0, ThreadWrapper, &iters_t1, 0, &dwThreadId1);
    if (hThreads[0] == NULL) {
        return 1;
    }

    hThreads[1] = CreateThread(NULL, 0, ThreadWrapper, &iters_t2, 0, &dwThreadId2);
    if (hThreads[1] == NULL) {
        CloseHandle(hThreads[0]);
        return 1;
    }

    size_t user_size = 64;
    size_t name_size = 128;
    wchar_t* username = (wchar_t*)malloc(user_size * sizeof(wchar_t));
    wchar_t* fullname = (wchar_t*)malloc(name_size * sizeof(wchar_t));

    if (username == NULL || fullname == NULL) {
        if (username) free(username);
        if (fullname) free(fullname);
        return 1;
    }

    wcscpy_s(username, user_size, L"User-3af11550");
    wcscpy_s(fullname, name_size, L"Антипов Алексей Романович");

    size_t user_len = wcslen(username);
    size_t name_len = wcslen(fullname);

    DWORD pid = GetCurrentProcessId();
    DWORD tid = GetCurrentThreadId();

    srand((unsigned int)time(NULL) ^ tid);

    for (int i = 0; i < 100; ++i) {
        if (i == 49) {
            TerminateThread(hThreads[1], 0);
        }

        wchar_t u_char = username[i % user_len];
        wchar_t f_char = fullname[i % name_len];

        wprintf(L"%lu - %lu - %d - %lc - %lc\n", pid, tid, i + 1, u_char, f_char);

        int delay = 350 + (rand() % 101);
        Sleep(delay);
    }

    WaitForMultipleObjects(2, hThreads, TRUE, INFINITE);

    free(username);
    free(fullname);
    CloseHandle(hThreads[0]);
    CloseHandle(hThreads[1]);

    system("pause");
    return 0;
}