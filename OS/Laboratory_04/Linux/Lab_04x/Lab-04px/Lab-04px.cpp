#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/syscall.h>
#include <wchar.h>
#include <locale.h>
#include <time.h>

void* Lab_04x(void* arg) {
    int iterations = *((int*)arg);
    if (iterations <= 0) return NULL;

    size_t user_size = 64;
    size_t name_size = 128;

    wchar_t* username = (wchar_t*)malloc(user_size * sizeof(wchar_t));
    wchar_t* fullname = (wchar_t*)malloc(name_size * sizeof(wchar_t));

    if (!username || !fullname) {
        if (username) free(username);
        if (fullname) free(fullname);
        return NULL;
    }

    wcscpy(username, L"User-3af11550");
    wcscpy(fullname, L"Антипов Алексей Романович");

    size_t user_len = wcslen(username);
    size_t name_len = wcslen(fullname);

    pid_t pid = getpid();
    pid_t tid = syscall(SYS_gettid);

    srand((unsigned int)time(NULL) ^ tid);

    for (int i = 0; i < iterations; ++i) {
        wchar_t u_char = username[i % user_len];
        wchar_t f_char = fullname[i % name_len];

        wprintf(L"%d - %d - %d - %lc - %lc\n", pid, tid, i + 1, u_char, f_char);

        int delay = 350000 + (rand() % 100001);
        usleep(delay);
    }

    free(username);
    free(fullname);
    return NULL;
}

int main() {
    setlocale(LC_ALL, "");

    pid_t main_pid = getpid();
    wprintf(L"%d\n", main_pid);

    int iters_main = 100;
    int iters_t1 = 50;
    int iters_t2 = 125;

    pthread_t thread1, thread2;

    if (pthread_create(&thread1, NULL, Lab_04x, &iters_t1) != 0) {
        return 1;
    }

    if (pthread_create(&thread2, NULL, Lab_04x, &iters_t2) != 0) {
        return 1;
    }

    Lab_04x(&iters_main);

    pthread_join(thread1, NULL);
    pthread_join(thread2, NULL);

    return 0;
}