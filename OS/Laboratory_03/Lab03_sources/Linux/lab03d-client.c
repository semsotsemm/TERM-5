#define _POSIX_C_SOURCE 200809L

#include <errno.h>
#include <inttypes.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include "lab03_common.h"

static uint64_t gcd(uint64_t a, uint64_t b) {
    while (b != 0) {
        uint64_t next = a % b;
        a = b;
        b = next;
    }
    return a;
}

int main(int argc, char** argv) {
    uint64_t m = 0, left = 0, right = 0;
    if (argc != 4 || !parse_positive(argv[1], &m) ||
        !parse_positive(argv[2], &left) ||
        !parse_positive(argv[3], &right) || left > right) {
        fprintf(stderr, "Usage: lab03d-client M L R (M>0, 1<=L<=R)\n");
        return 1;
    }

    uint64_t* found = NULL;
    size_t used = 0, capacity = 0;
    for (uint64_t n = left;; ++n) {
        if (gcd(n, m) == 1) {
            if (used == capacity) {
                size_t next = capacity ? capacity * 2 : 64;
                if (next < capacity || next > SIZE_MAX / sizeof(*found)) {
                    fprintf(stderr, "Result buffer too large\n");
                    free(found);
                    return 1;
                }
                uint64_t* grown = realloc(found, next * sizeof(*found));
                if (!grown) { perror("realloc"); free(found); return 1; }
                found = grown;
                capacity = next;
            }
            found[used++] = n;
        }
        if (n == right) break;
    }

    // Diagnostic pause gives time to inspect /proc/<pid>/fd. Do not use it
    // for the actual timing measurement.
    const char* pause_text = getenv("LAB03_PAUSE_MS");
    if (pause_text) {
        uint64_t ms = 0;
        if (parse_positive(pause_text, &ms) && ms <= 30000) {
            struct timespec delay = {(time_t)(ms / 1000),
                                     (long)(ms % 1000) * 1000000L};
            while (nanosleep(&delay, &delay) == -1 && errno == EINTR) {}
        }
    }

    for (size_t i = 0; i < used; ++i) printf("%" PRIu64 " ", found[i]);
    if (fflush(stdout) == EOF) {
        perror("fflush");
        free(found);
        return 1;
    }
    free(found);
    return 0;
}
