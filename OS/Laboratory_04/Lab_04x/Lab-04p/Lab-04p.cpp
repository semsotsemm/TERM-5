#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

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
    for (int i = args->start; i <= args->end; i++) {
        if (isPrime(i)) {
            globalPrimes[globalIndex] = i;
            int temp = globalIndex;
            Sleep(1);

            globalIndex = temp + 1;
        }
    }
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
            for (int j = 0; j < i; j++) {
                TerminateThread(hThreads[j], 0);
                CloseHandle(hThreads[j]);
            }
            free(globalPrimes);
            free(hThreads);
            free(tArgs);
            return 1;
        }
    }

    WaitForMultipleObjects(numThreads, hThreads, TRUE, INFINITE);

    qsort(globalPrimes, globalIndex, sizeof(int), compare);

    for (int i = 0; i < globalIndex; i++) {
        if (globalPrimes[i] != 0) {
            printf("%d ", globalPrimes[i]);
        }
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