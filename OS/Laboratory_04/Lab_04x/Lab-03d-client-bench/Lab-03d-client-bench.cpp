#include <iostream>
#include <windows.h>
#include <string>
#include <sstream>

using namespace std;

bool isPrime(int n) {
    if (n <= 1) return false;
    for (int i = 2; i * i <= n; ++i) {
        if (n % i == 0) return false;
    }
    return true;
}

int main(int argc, char* argv[])
{
    if (argc < 4)
    {
        return 1;
    }

    long long L = stoll(argv[2]);
    long long R = stoll(argv[3]);

    stringstream ss;
    for (long long n = L; n <= R; n++)
    {
        if (isPrime((int)n))
        {
            ss << n << " ";
        }
    }

    string result = ss.str();

    HANDLE h_out = GetStdHandle(STD_OUTPUT_HANDLE);
    if (h_out == INVALID_HANDLE_VALUE || h_out == NULL)
    {
        ExitProcess(GetLastError());
    }

    size_t offset = 0;

    while (offset < result.size()) {
        DWORD remaining = (DWORD)(result.size() - offset);
        DWORD written = 0;

        if (!WriteFile(h_out, result.data() + offset,
            remaining, &written, NULL)) {
            return 1;
        }

        if (written == 0) return 1;
        offset += written;
    }

    return 0;
}