#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

int* globalPrimes = NULL;
int globalIndex = 0;
DWORD tlsIndex = TLS_OUT_OF_INDEXES;

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
    int capacity = args->end - args->start + 1;
    if (capacity <= 0) return 0;

    int* buffer = (int*)malloc((size_t)capacity * sizeof(int));
    if (!buffer) return ERROR_OUTOFMEMORY;

    if (!TlsSetValue(tlsIndex, buffer)) {
        DWORD error = GetLastError();
        free(buffer);
        return error;
    }

    int* local = (int*)TlsGetValue(tlsIndex);
    if (!local) {
        TlsSetValue(tlsIndex, NULL);
        free(buffer);
        return ERROR_INVALID_DATA;
    }

    int count = 0;
    for (int n = args->start; n <= args->end; ++n) {
        if (isPrime(n)) local[count++] = n;
    }

    for (int i = 0; i < count; ++i) {
        globalPrimes[globalIndex] = local[i];
        int oldIndex = globalIndex;
        globalIndex = oldIndex + 1;
    }

    free(buffer);
    if (!TlsSetValue(tlsIndex, NULL)) return GetLastError();
    return 0;
}

int compare(const void* a, const void* b) {
    return (*(int*)a - *(int*)b);
}

int main(int argc, char* argv[]) {
    if (argc != 4) return 1;

    int numThreads = atoi(argv[1]);
    int lower = atoi(argv[2]);
    int upper = atoi(argv[3]);

    if (numThreads <= 0 || lower > upper) return 1;

    int maxPossiblePrimes = upper - lower + 1;
    globalPrimes = (int*)malloc(maxPossiblePrimes * sizeof(int));
    if (!globalPrimes) return 1;

    HANDLE* hThreads = (HANDLE*)malloc(numThreads * sizeof(HANDLE));
    ThreadArgs* tArgs = (ThreadArgs*)malloc(numThreads * sizeof(ThreadArgs));

    if (!hThreads || !tArgs) {
        if (globalPrimes) free(globalPrimes);
        if (hThreads) free(hThreads);
        if (tArgs) free(tArgs);
        return 1;
    }

    int rangeSize = upper - lower + 1;
    int chunkSize = rangeSize / numThreads;
    int remainder = rangeSize % numThreads;
    int currentStart = lower;
    tlsIndex = TlsAlloc();
    if (tlsIndex == TLS_OUT_OF_INDEXES) {
        fprintf(stderr, "TlsAlloc failed: %lu\n", GetLastError());
        free(globalPrimes);
        free(hThreads);
        free(tArgs);
        return 1;
    }

    for (int i = 0; i < numThreads; i++) {
        tArgs[i].start = currentStart;
        tArgs[i].end = currentStart + chunkSize - 1 + (i < remainder ? 1 : 0);
        currentStart = tArgs[i].end + 1;

        hThreads[i] = CreateThread(NULL, 0, L4Primes, &tArgs[i], 0, NULL);
        if (hThreads[i] == NULL) {
            DWORD error = GetLastError();
            fprintf(stderr, "CreateThread failed: %lu\n", error);

            for (int j = 0; j < i; ++j) {
                if (WaitForSingleObject(hThreads[j], INFINITE) != WAIT_OBJECT_0) {
                    fprintf(stderr, "WaitForSingleObject failed: %lu\n", GetLastError());
                    return 1;
                }
                CloseHandle(hThreads[j]);
            }

            TlsFree(tlsIndex);
            free(globalPrimes);
            free(hThreads);
            free(tArgs);
            return 1;
        }
    }

    bool workersOk = true;

    for (int i = 0; i < numThreads; ++i) {
        if (WaitForSingleObject(hThreads[i], INFINITE) != WAIT_OBJECT_0) {
            fprintf(stderr, "WaitForSingleObject failed: %lu\n", GetLastError());
            return 1;
        }

        DWORD exitCode;
        if (!GetExitCodeThread(hThreads[i], &exitCode)) {
            fprintf(stderr, "GetExitCodeThread failed: %lu\n", GetLastError());
            workersOk = false;
        }
        else if (exitCode != 0) {
            fprintf(stderr, "Worker %d failed: %lu\n", i, exitCode);
            workersOk = false;
        }
    }

    if (!workersOk) {
        for (int i = 0; i < numThreads; ++i) {
            CloseHandle(hThreads[i]);
        }
        TlsFree(tlsIndex);
        free(globalPrimes);
        free(hThreads);
        free(tArgs);
        return 1;
    }

    qsort(globalPrimes, globalIndex, sizeof(int), compare);

    printf("Count: %d\n", globalIndex);

    for (int i = 0; i < globalIndex; i++) {
        if (globalPrimes[i] != 0) {
            printf("%d ", globalPrimes[i]);
        }
    }
    printf("\n");

    for (int i = 0; i < numThreads; i++) {
        CloseHandle(hThreads[i]);
    }

    TlsFree(tlsIndex);

    free(globalPrimes);
    free(hThreads);
    free(tArgs);

    return 0;
}