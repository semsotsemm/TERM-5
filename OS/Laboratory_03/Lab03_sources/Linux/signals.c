#define _POSIX_C_SOURCE 200809L

#include <signal.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

static void on_usr1(int signal_number) {
    (void)signal_number;
    static const char message[] = "Received USR1 signal\n";
    write(STDOUT_FILENO, message, sizeof(message) - 1);
}

static void on_usr2(int signal_number) {
    (void)signal_number;
    static const char message[] = "Received USR2 signal\n";
    write(STDOUT_FILENO, message, sizeof(message) - 1);
}

int main(void) {
    struct sigaction action;
    memset(&action, 0, sizeof(action));
    sigemptyset(&action.sa_mask);
    action.sa_handler = on_usr1;
    if (sigaction(SIGUSR1, &action, NULL) == -1) { perror("SIGUSR1"); return 1; }
    action.sa_handler = on_usr2;
    if (sigaction(SIGUSR2, &action, NULL) == -1) { perror("SIGUSR2"); return 1; }
    action.sa_handler = SIG_IGN;
    if (sigaction(SIGINT, &action, NULL) == -1) { perror("SIGINT"); return 1; }

    printf("Program running with PID %ld\n", (long)getpid());
    fflush(stdout);
    for (;;) pause();
}
