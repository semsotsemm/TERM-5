#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <locale.h>

#define MAX_PRIMES 100000

int globalPrimes[MAX_PRIMES];
int globalIndex = 0;
DWORD tlsIndex;

typedef struct {
    int start;
    int end;
} ThreadArgs;

bool isPrime(int n) {
    if (n <= 1) return false;
    for (int i = 2; i * i <= n; i++) {
        if (n % i == 0) return false;
    }
    return true;
}

DWORD WINAPI L4Primes(LPVOID lpParam) {
    ThreadArgs* args = (ThreadArgs*)lpParam;
    int start = args->start;
    int end = args->end;

    int capacity = end - start + 1;
    if (capacity <= 0) return 0;

    int* localBuffer = (int*)malloc(capacity * sizeof(int));
    if (!localBuffer) return 0;

    int localCount = 0;

    TlsSetValue(tlsIndex, localBuffer);

    for (int i = start; i <= end; i++) {
        if (isPrime(i)) {
            int* buf = (int*)TlsGetValue(tlsIndex);
            buf[localCount] = i;
            localCount++;
        }
    }

    int* finalBuf = (int*)TlsGetValue(tlsIndex);
    for (int i = 0; i < localCount; i++) {
        globalPrimes[globalIndex] = finalBuf[i];
        int temp = globalIndex;
        Sleep(1);
        globalIndex = temp + 1;
    }

    free(finalBuf);
    return 0;
}

int main() {
    setlocale(LC_ALL, "Russian");

    tlsIndex = TlsAlloc();
    if (tlsIndex == TLS_OUT_OF_INDEXES) {
        return 1;
    }

    ThreadArgs args1 = { 2, 5000 };
    ThreadArgs args2 = { 5001, 10000 };
    ThreadArgs args3 = { 10001, 15000 };

    HANDLE hThreads[3];
    hThreads[0] = CreateThread(NULL, 0, L4Primes, &args1, 0, NULL);
    hThreads[1] = CreateThread(NULL, 0, L4Primes, &args2, 0, NULL);
    hThreads[2] = CreateThread(NULL, 0, L4Primes, &args3, 0, NULL);

    if (hThreads[0] == NULL || hThreads[1] == NULL || hThreads[2] == NULL) {
        return 1;
    }

    WaitForMultipleObjects(3, hThreads, TRUE, INFINITE);

    wprintf(L"Index: %d\n", globalIndex);

    CloseHandle(hThreads[0]);
    CloseHandle(hThreads[1]);
    CloseHandle(hThreads[2]);
    TlsFree(tlsIndex);

    system("pause");
    return 0;
}