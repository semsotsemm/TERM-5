#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <pthread.h>
#include <unistd.h>

__thread int localCount = 0;

int* globalPrimes = NULL;
int globalIndex = 0;
pthread_once_t once_control = PTHREAD_ONCE_INIT;
pthread_key_t tlsKey;

void init_tls() {
    pthread_key_create(&tlsKey, free);
}

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

void* FindPrimes(void* arg) {
    pthread_once(&once_control, init_tls);

    ThreadArgs* args = (ThreadArgs*)arg;
    int capacity = args->end - args->start + 1;
    if (capacity <= 0) return NULL;

    int* localBuffer = (int*)malloc(capacity * sizeof(int));
    pthread_setspecific(tlsKey, localBuffer);
    localCount = 0;

    for (int i = args->start; i <= args->end; i++) {
        if (isPrime(i)) {
            localBuffer[localCount++] = i;
        }
    }

    for (int i = 0; i < localCount; i++) {
        globalPrimes[globalIndex] = localBuffer[i];
        int temp = globalIndex;
        usleep(1000);
        globalIndex = temp + 1;
    }

    return NULL;
}

int compare(const void* a, const void* b) {
    return (*(int*)a - *(int*)b);
}

int main(int argc, char* argv[]) {
    if (argc != 4) return 1;

    int numThreads = atoi(argv[1]);
    int lower = atoi(argv[2]);
    int upper = atoi(argv[3]);

    if (numThreads <= 0 || lower < upper) return 1;

    int maxPossiblePrimes = upper - lower + 1;
    globalPrimes = (int*)malloc(maxPossiblePrimes * sizeof(int));
    if (!globalPrimes) return 1;

    pthread_t* threads = (pthread_t*)malloc(numThreads * sizeof(pthread_t));
    ThreadArgs* tArgs = (ThreadArgs*)malloc(numThreads * sizeof(ThreadArgs));

    if (!threads || !tArgs) {
        if (globalPrimes) free(globalPrimes);
        if (threads) free(threads);
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

        if (pthread_create(&threads[i], NULL, FindPrimes, &tArgs[i]) != 0) {
            return 1;
        }
    }

    for (int i = 0; i < numThreads; i++) {
        pthread_join(threads[i], NULL);
    }

    qsort(globalPrimes, globalIndex, sizeof(int), compare);

    for (int i = 0; i < globalIndex; i++) {
        if (globalPrimes[i] != 0) {
            printf("%d ", globalPrimes[i]);
        }
    }
    printf("\n");

    free(globalPrimes);
    free(threads);
    free(tArgs);

    return 0;
}