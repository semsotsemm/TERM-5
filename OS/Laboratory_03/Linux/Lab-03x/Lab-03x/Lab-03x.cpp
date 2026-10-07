#include <iostream>
#include <cstdlib>
#include <unistd.h>

using namespace std;

int main(int argc, char* argv[])
{
    int iterations = 0;

    if (argc > 1)
    {
        iterations = atoi(argv[1]);
    }
    else
    {
        char* env_iter = getenv("ITER_NUM");
        if (env_iter != NULL)
        {
            iterations = atoi(env_iter);
        }
        else
        {
            cerr << "Error: No ITER_NUM or command line argument provided.\n";
            exit(139);
        }
    }

    cout << "[PID " << getpid() << "] Starting with " << iterations << " iterations.\n";

    for (int i = 0; i < iterations; ++i)
    {
        cout << "[PID " << getpid() << "] Iteration: " << i + 1 << "\n";
        usleep(500000);
    }

    return 0;
}