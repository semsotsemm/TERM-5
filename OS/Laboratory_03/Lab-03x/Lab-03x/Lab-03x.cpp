#define _CRT_SECURE_NO_WARNINGS
#include <iostream>
#include <string>
#include <cstdlib>
#include <windows.h>

using namespace std;

int main(int argc, char* argv[])
{
    string iteration_line;

    if (argc >= 2)
    {
        iteration_line = argv[1];
        cout << "The number of iterations was successfully obtained from the command-line argument. Number of iteration: " << iteration_line << "\n";
    }
    else
    {
        const char* env_parameter = getenv("ITER_NUM");
        if (env_parameter != nullptr)
        {
            iteration_line = env_parameter;
            cout << "The number of iterations was successfully obtained from the environment variable. Number of iteration: " << iteration_line << "\n";
        }
        else
        {
            cerr << "Failed to obtain the number of loop iterations from either the command-line argument or the environment variable. Set an environment variable named ITER_NUM or pass the number of iterations using the command-line argument -i n\n";
            ExitProcess(0xC0000005);
        }
    }

    int iteration = 0;
    try 
    {
        iteration = stoi(iteration_line);
    }
    catch(...) 
    {
        ExitProcess(0xC0000005);
    }

    DWORD pid = GetCurrentProcessId();

    for (int i = 0; i < iteration; i++)
    {
        cout << "PID: " << pid << " | Iteration: " << i + 1 << "\n";
        Sleep(500);
    }
    return 0;
}