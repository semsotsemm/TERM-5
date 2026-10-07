#include <iostream>
#include <unistd.h>
#include <sys/wait.h>
#include <cstdlib>

using namespace std;

int main()
{
    pid_t pid1, pid2;

    cout << "Parent PID: " << getpid() << "\n";
    
    pid1 = fork();
    if (pid1 == 0)
    {
        execl("./Lab-03x", "Lab-03x", "10", NULL);

        cerr << "Failed to start first child.\n";
        exit(1);
    }

    setenv("ITER_NUM", "15", 1);

    pid2 = fork();
    if (pid2 == 0)
    {
        execl("./Lab-03x", "Lab-03x", NULL);

        cerr << "Failed to start second child.\n";
        exit(1);
    }

    int status;
    if (pid1 > 0) waitpid(pid1, &status, 0);
    if (pid2 > 0) waitpid(pid2, &status, 0);

    cout << "Parent: All child processes have finished.\n";
    return 0;
}