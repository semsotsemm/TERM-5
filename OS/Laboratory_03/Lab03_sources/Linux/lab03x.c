#define _POSIX_C_SOURCE 200809L

#include <errno.h>
#include <inttypes.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>

#include "lab03_common.h"

int main(int argc, char** argv) {
    const char* source = argc > 1 ? argv[1] : getenv("ITER_NUM");
    uint64_t count = 0;
    if (!parse_positive(source, &count)) {
        fprintf(stderr, "Expected positive iteration count in argv[1] or ITER_NUM\n");
        return 1;  // POSIX exit codes cannot represent Windows 0xC0000005.
    }

    printf("Iterations: %" PRIu64 "\n", count);
    fflush(stdout);
    for (uint64_t i = 0; i < count; ++i) {
        printf("PID=%ld iteration=%" PRIu64 "\n", (long)getpid(), i + 1);
        fflush(stdout);
        struct timespec delay = {0, 500000000L};
        while (nanosleep(&delay, &delay) == -1 && errno == EINTR) {}
    }
    return 0;
}
