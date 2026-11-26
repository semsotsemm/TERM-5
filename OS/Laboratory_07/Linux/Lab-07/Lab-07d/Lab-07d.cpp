#include <iostream>
#include <unistd.h>
#include <sys/wait.h>

using namespace std;

int main()
{
    setlocale(LC_CTYPE, "");

    const char* start_command = "./Lab-07x";
    const char* first_thread_work_time = "60";
    const char* second_thread_work_time = "120";

    pid_t first_pid = fork();
    if (first_pid == 0)
    {
        execl(start_command, "Lab-07x", first_thread_work_time, nullptr);
        return 1;
    }

    pid_t second_pid = fork();
    if (second_pid == 0)
    {
        execl(start_command, "Lab-07x", second_thread_work_time, nullptr);
        return 1;
    }

    waitpid(first_pid, nullptr, 0);
    waitpid(second_pid, nullptr, 0);

    return 0;
}