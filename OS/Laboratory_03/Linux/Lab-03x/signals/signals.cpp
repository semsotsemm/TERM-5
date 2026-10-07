#include <iostream>
#include <unistd.h>
#include <signal.h>

using namespace std;

void signal_handler(int signum) {
    if (signum == SIGUSR1) {
        cout << "Received USR1 signal\n";
    }
    else if (signum == SIGUSR2) {
        cout << "Received USR2 signal\n";
    }
}

int main() {
    signal(SIGUSR1, signal_handler);
    signal(SIGUSR2, signal_handler);
    
    signal(SIGINT, SIG_IGN);

    cout << "Program running with PID " << getpid() << "\n";

    while (true) {
        pause();
    }

    return 0;
}