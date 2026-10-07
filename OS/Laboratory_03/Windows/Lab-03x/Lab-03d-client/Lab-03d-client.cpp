#include <iostream>
#include <windows.h>
#include <string>
#include <sstream>

using namespace std;

// Функция нахождения нод
long long GetGreatestCommonDivisor(long long a, long long b) 
{
    while (b) 
    {
        a %= b;
        swap(a, b);
    }
    return a;
}

int main(int argc, char* argv[]) 
{
    if (argc < 4) 
    {
        return 1;
    }

    long long M = stoll(argv[1]);
    long long L = stoll(argv[2]);
    long long R = stoll(argv[3]);

    stringstream ss;
    for (long long n = L; n <= R; n++) 
    {
        if (GetGreatestCommonDivisor(n, M) == 1) 
        {
            ss << n << " ";
        }
    }

    string result = ss.str();

    HANDLE h_out = GetStdHandle(STD_OUTPUT_HANDLE);
    if (h_out == INVALID_HANDLE_VALUE)
    {
        ExitProcess(GetLastError());
    }

    DWORD written;
    if (!result.empty()) 
    {
        WriteFile(h_out, result.c_str(), result.length(), &written, NULL);
    }

    return 0;
}