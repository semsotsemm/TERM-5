#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

__declspec(thread) int* localBuffer = NULL;
__declspec(thread) int localCount = 0;

int* globalPrimes = NULL;
int globalIndex = 0;

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

DWORD WINAPI FindPrimes(LPVOID lpParam) {
    ThreadArgs* args = (ThreadArgs*)lpParam;
    int capacity = args->end - args->start + 1;

    if (capacity <= 0) return 0;

    localBuffer = (int*)malloc(capacity * sizeof(int));
    if (!localBuffer) return ERROR_OUTOFMEMORY;

    localCount = 0;

    for (int i = args->start; i <= args->end; i++) {
        if (isPrime(i)) {
            localBuffer[localCount++] = i;
        }
    }

    for (int i = 0; i < localCount; ++i) {
        globalPrimes[globalIndex] = localBuffer[i];
        int oldIndex = globalIndex;
        globalIndex = oldIndex + 1;
    }

    free(localBuffer);
    localBuffer = NULL;
    localCount = 0;
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


    for (int i = 0; i < numThreads; i++) {
        tArgs[i].start = currentStart;
        tArgs[i].end = currentStart + chunkSize - 1 + (i < remainder ? 1 : 0);
        currentStart = tArgs[i].end + 1;

        hThreads[i] = CreateThread(NULL, 0, FindPrimes, &tArgs[i], 0, NULL);
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
        free(globalPrimes);
        free(hThreads);
        free(tArgs);
        return 1;
    }


    qsort(globalPrimes, globalIndex, sizeof(int), compare);

    printf("Count: %d\n", globalIndex);
    for (int i = 0; i < globalIndex; ++i) {
        printf("%d ", globalPrimes[i]);
    }
    printf("\n");

    for (int i = 0; i < numThreads; i++) {
        CloseHandle(hThreads[i]);
    }

    free(globalPrimes);
    free(hThreads);
    free(tArgs);

    return 0;
}