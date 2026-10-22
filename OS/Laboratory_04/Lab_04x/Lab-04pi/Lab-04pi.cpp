#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <atomic>

std::atomic_bool keep_running{ true };
double total_pi = 0.0;
long long current_n = 0;
long long CHUNK_SIZE = 5000;
LPVOID main_fiber;

BOOL WINAPI CtrlHandler(DWORD fdwCtrlType) {
    if (fdwCtrlType == CTRL_C_EVENT) {
        keep_running.store(false, std::memory_order_relaxed);
        return TRUE;
    }
    return FALSE;
}

void WINAPI FiberFunc(LPVOID lpParameter) {
    while (keep_running.load(std::memory_order_relaxed)) {
        long long start = current_n;
        current_n += CHUNK_SIZE;

        double partial_sum = 0.0;
        for (long long i = start; i < start + CHUNK_SIZE; i++) {
            if (!keep_running.load(std::memory_order_relaxed)) break;

            double term = 1.0 / (2.0 * i + 1.0);
            if (i % 2 != 0) {
                term = -term;
            }
            partial_sum += term;
        }

        total_pi += partial_sum;
        SwitchToFiber(main_fiber);
    }
    SwitchToFiber(main_fiber);
}

int main(int argc, char* argv[]) {
    if (argc != 2) return 1;

    int num_fibers = atoi(argv[1]);
    if (num_fibers <= 0) return 1;

    if (!SetConsoleCtrlHandler(CtrlHandler, TRUE)) {
        return 1;
    }

    main_fiber = ConvertThreadToFiber(NULL);
    if (!main_fiber) return 1;

    LPVOID* fibers = (LPVOID*)malloc(num_fibers * sizeof(LPVOID));
    if (!fibers) return 1;

    for (int i = 0; i < num_fibers; i++) {
        fibers[i] = CreateFiber(0, FiberFunc, NULL);
        if (!fibers[i]) {
            keep_running.store(false, std::memory_order_relaxed);
        }
    }

    while (keep_running.load(std::memory_order_relaxed)) {
        for (int i = 0; i < num_fibers; i++) {
            if (!keep_running.load(std::memory_order_relaxed)) break;
            SwitchToFiber(fibers[i]);
        }
    }

    printf("%.15f\n", total_pi * 4.0);

    for (int i = 0; i < num_fibers; i++) {
        if (fibers[i]) {
            DeleteFiber(fibers[i]);
        }
    }
    free(fibers);

    return 0;
}