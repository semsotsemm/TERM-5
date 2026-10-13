#define _CRT_SECURE_NO_WARNINGS
#include <iostream>
#include <string>
#include <cstdlib>
#include <windows.h>

using namespace std;

int main(int argc, char* argv[])
{
    int iteration_count;

    if (argc > 1)
    {
        iteration_count = stoi(argv[1]);
        cout << "The number of iterations was successfully obtained from the command-line argument.\nNumber of iteration: " << iteration_count << "\n";
    }
    else
    {
        const char* env_parameter = getenv("ITER_NUM");
        if (env_parameter != nullptr)
        {
            iteration_count = stoi(env_parameter);
            cout << "The number of iterations was successfully obtained from the environment variable.\nNumber of iteration: " << iteration_count << "\n";
        }
        else
        {
            cerr << "Failed to obtain the number of loop iterations from either the command-line argument or the environment variable.\nSet an environment variable named ITER_NUM or pass the number of iterations using the command-line argument -i n\n";
            ExitProcess(1);
        }
    }

    DWORD pid = GetCurrentProcessId();

    for (int i = 0; i < iteration_count; i++)
    {
        cout << "PID: " << pid << " | Iteration: " << i + 1 << "\n";
        Sleep(500);
    }
    return 0;
}