#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <locale.h>

void Lab_04x(int iterations) {
    if (iterations <= 0) {
        printf("Ошибка: Количество итераций должно быть положительным числом (> 0).\n");
        return;
    }

    size_t user_size = 64;
    size_t name_size = 128;

    wchar_t* username = (wchar_t*)malloc(user_size * sizeof(wchar_t));
    wchar_t* fullname = (wchar_t*)malloc(name_size * sizeof(wchar_t));

    if (username == NULL || fullname == NULL) {
        printf("Ошибка: Не удалось выделить память.\n");
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

int main() {
    setlocale(LC_ALL, "Russian");

    printf("--- Начало работы функции Lab_04x ---\n");

    Lab_04x(30);

    printf("--- Функция завершила работу ---\n");

    system("pause");
    return 0;
}