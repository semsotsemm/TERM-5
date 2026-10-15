#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <signal.h>
#include <ucontext.h>
#include <unistd.h>

volatile sig_atomic_t keep_running = 1;
double total_pi = 0.0;
long long current_n = 0;
long long CHUNK_SIZE = 4000;

ucontext_t main_ctx;
ucontext_t* fiber_ctxs;
int num_fibers;
int current_fiber = 0;

void sigint_handler(int signum) {
    keep_running = 0;
}

void fiber_func() {
    while (keep_running) {
        long long start = current_n;
        current_n += CHUNK_SIZE;

        double partial_sum = 0.0;
        for (long long i = start; i < start + CHUNK_SIZE; i++) {
            if (!keep_running) break;

            double term = 1.0 / (2.0 * i + 1.0);
            if (i % 2 != 0) {
                term = -term;
            }
            partial_sum += term;
        }

        total_pi += partial_sum;
        swapcontext(&fiber_ctxs[current_fiber], &main_ctx);
    }
}

int main(int argc, char* argv[]) {
    if (argc != 2) return 1;

    num_fibers = atoi(argv[1]);
    if (num_fibers <= 0) return 1;

    struct sigaction sa;
    sa.sa_handler = sigint_handler;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;
    if (sigaction(SIGINT, &sa, NULL) == -1) {
        return 1;
    }

    fiber_ctxs = (ucontext_t*)malloc(num_fibers * sizeof(ucontext_t));
    char** stacks = (char**)malloc(num_fibers * sizeof(char*));

    if (!fiber_ctxs || !stacks) return 1;

    for (int i = 0; i < num_fibers; i++) {
        getcontext(&fiber_ctxs[i]);
        stacks[i] = (char*)malloc(64 * 1024);
        fiber_ctxs[i].uc_stack.ss_sp = stacks[i];
        fiber_ctxs[i].uc_stack.ss_size = 64 * 1024;
        fiber_ctxs[i].uc_link = &main_ctx;
        makecontext(&fiber_ctxs[i], fiber_func, 0);
    }

    while (keep_running) {
        for (int i = 0; i < num_fibers; i++) {
            if (!keep_running) break;
            current_fiber = i;
            swapcontext(&main_ctx, &fiber_ctxs[i]);
        }
    }

    printf("%.15f\n", total_pi * 4.0);

    for (int i = 0; i < num_fibers; i++) {
        free(stacks[i]);
    }
    free(stacks);
    free(fiber_ctxs);

    return 0;
}