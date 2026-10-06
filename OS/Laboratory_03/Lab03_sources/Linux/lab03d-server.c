#define _POSIX_C_SOURCE 200809L

#include <ctype.h>
#include <errno.h>
#include <inttypes.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <time.h>
#include <unistd.h>

#include "lab03_common.h"

struct child {
    pid_t pid;
    int read_fd;
};

static int append_number(uint64_t** values, size_t* used, size_t* capacity,
                         uint64_t number) {
    if (*used == *capacity) {
        size_t next = *capacity ? *capacity * 2 : 64;
        if (next < *capacity || next > SIZE_MAX / sizeof(**values)) return 0;
        uint64_t* grown = realloc(*values, next * sizeof(**values));
        if (!grown) return 0;
        *values = grown;
        *capacity = next;
    }
    (*values)[(*used)++] = number;
    return 1;
}

static int read_results(int fd, uint64_t** values, size_t* used,
                        size_t* capacity) {
    char* data = NULL;
    size_t length = 0, allocated = 0;
    char block[4096];
    for (;;) {
        ssize_t bytes = read(fd, block, sizeof(block));
        if (bytes == -1 && errno == EINTR) continue;
        if (bytes == -1) { perror("read"); free(data); return 0; }
        if (bytes == 0) break;
        size_t size = (size_t)bytes;
        if (length > SIZE_MAX - size - 1) { free(data); return 0; }
        if (length + size + 1 > allocated) {
            size_t next = allocated ? allocated : 4096;
            while (next < length + size + 1) {
                if (next > SIZE_MAX / 2) { free(data); return 0; }
                next *= 2;
            }
            char* grown = realloc(data, next);
            if (!grown) { free(data); return 0; }
            data = grown;
            allocated = next;
        }
        memcpy(data + length, block, size);
        length += size;
    }
    if (data) data[length] = '\0';
    char* cursor = data;
    while (cursor && *cursor) {
        while (isspace((unsigned char)*cursor)) ++cursor;
        if (!*cursor) break;
        char* end = NULL;
        errno = 0;
        unsigned long long number = strtoull(cursor, &end, 10);
        if (errno == ERANGE || end == cursor ||
            (*end && !isspace((unsigned char)*end)) ||
            !append_number(values, used, capacity, (uint64_t)number)) {
            fprintf(stderr, "Invalid or oversized client output\n");
            free(data);
            return 0;
        }
        cursor = end;
    }
    free(data);
    return 1;
}

static int compare_numbers(const void* a, const void* b) {
    uint64_t x = *(const uint64_t*)a, y = *(const uint64_t*)b;
    return (x > y) - (x < y);
}

static double elapsed_ms(const struct timespec* start,
                         const struct timespec* finish) {
    return (finish->tv_sec - start->tv_sec) * 1000.0 +
           (finish->tv_nsec - start->tv_nsec) / 1000000.0;
}

int main(int argc, char** argv) {
    uint64_t k = 0, m = 0, left = 0, right = 0;
    if (argc != 5 || !parse_positive(argv[1], &k) ||
        !parse_positive(argv[2], &m) || !parse_positive(argv[3], &left) ||
        !parse_positive(argv[4], &right) || left > right ||
        right - left == UINT64_MAX || k > right - left + 1 ||
        k > SIZE_MAX / sizeof(struct child)) {
        fprintf(stderr, "Usage: lab03d-server K M L R (1<=K<=R-L+1)\n");
        return 1;
    }
    struct child* children = calloc((size_t)k, sizeof(*children));
    if (!children) { perror("calloc"); return 1; }

    struct timespec start, finish;
    clock_gettime(CLOCK_MONOTONIC, &start);
    uint64_t base = (right - left + 1) / k;
    uint64_t remainder = (right - left + 1) % k;
    uint64_t cursor = left;
    size_t launched = 0;
    int failed = 0;

    // Each child gets its own anonymous pipe. All children run concurrently.
    for (uint64_t i = 0; i < k; ++i) {
        uint64_t part_length = base + (i == 1 ? remainder : 0);
        uint64_t part_right = cursor + part_length - 1;
        int channel[2];
        if (pipe(channel) == -1) { perror("pipe"); failed = 1; break; }
        pid_t pid = fork();
        if (pid == -1) {
            perror("fork");
            close(channel[0]); close(channel[1]);
            failed = 1; break;
        }
        if (pid == 0) {
            close(channel[0]);
            for (size_t j = 0; j < launched; ++j) close(children[j].read_fd);
            if (dup2(channel[1], STDOUT_FILENO) == -1) {
                perror("dup2"); _exit(127);
            }
            close(channel[1]);
            char m_text[32], l_text[32], r_text[32];
            snprintf(m_text, sizeof(m_text), "%" PRIu64, m);
            snprintf(l_text, sizeof(l_text), "%" PRIu64, cursor);
            snprintf(r_text, sizeof(r_text), "%" PRIu64, part_right);
            execl("./lab03d-client", "lab03d-client", m_text, l_text,
                  r_text, (char*)NULL);
            perror("execl");
            _exit(127);
        }
        close(channel[1]);
        children[launched++] = (struct child){pid, channel[0]};
        printf("Child PID=%ld range=[%" PRIu64 ", %" PRIu64 "]\n",
               (long)pid, cursor, part_right);
        fflush(stdout);
        cursor = part_right + 1;
    }

    uint64_t* all = NULL;
    size_t used = 0, capacity = 0;
    for (size_t i = 0; i < launched; ++i) {
        if (!read_results(children[i].read_fd, &all, &used, &capacity)) failed = 1;
        close(children[i].read_fd);
        int status = 0;
        while (waitpid(children[i].pid, &status, 0) == -1) {
            if (errno == EINTR) continue;
            perror("waitpid"); failed = 1; break;
        }
        if (!WIFEXITED(status) || WEXITSTATUS(status) != 0) failed = 1;
    }
    clock_gettime(CLOCK_MONOTONIC, &finish);

    if (used) qsort(all, used, sizeof(*all), compare_numbers);
    printf("M=%" PRIu64 " range=[%" PRIu64 ", %" PRIu64 "] K=%" PRIu64 "\n",
           m, left, right, k);
    printf("Total found=%zu\nNumbers: ", used);
    for (size_t i = 0; i < used; ++i) printf("%" PRIu64 " ", all[i]);
    printf("\nElapsed ms=%.3f\n", elapsed_ms(&start, &finish));
    free(all);
    free(children);
    return failed ? 1 : 0;
}
