#define _POSIX_C_SOURCE 200809L

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

static int wait_child(pid_t pid) {
    int status = 0;
    while (waitpid(pid, &status, 0) == -1) {
        if (errno == EINTR) continue;
        perror("waitpid");
        return 1;
    }
    if (!WIFEXITED(status) || WEXITSTATUS(status) != 0) {
        fprintf(stderr, "Child %ld failed; raw status=%d\n", (long)pid, status);
        return 1;
    }
    return 0;
}

int main(void) {
    pid_t first = fork();
    if (first == -1) { perror("fork first"); return 1; }
    if (first == 0) {
        execl("./lab03x", "lab03x", "20", (char*)NULL);
        perror("execl first");
        _exit(127);
    }
    printf("First child PID=%ld: argv count 20\n", (long)first);
    fflush(stdout);

    // The variable is set in the parent immediately before the second fork.
    if (setenv("ITER_NUM", "18", 1) == -1) {
        perror("setenv");
        wait_child(first);
        return 1;
    }
    pid_t second = fork();
    if (second == -1) {
        perror("fork second");
        wait_child(first);
        return 1;
    }
    if (second == 0) {
        execl("./lab03x", "lab03x", (char*)NULL);
        perror("execl second");
        _exit(127);
    }
    printf("Second child PID=%ld: ITER_NUM=18\n", (long)second);
    fflush(stdout);

    // Both children were started before either waitpid call.
    int failed = wait_child(first);
    failed |= wait_child(second);
    return failed ? 1 : 0;
}
