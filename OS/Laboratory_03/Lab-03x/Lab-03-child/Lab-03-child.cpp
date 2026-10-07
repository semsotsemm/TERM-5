#include <iostream>
#include <windows.h>

using namespace std;

int main()
{
    STARTUPINFOA si;
    ZeroMemory(&si, sizeof(si));

    GetStartupInfoA(&si);

    if (si.lpTitle != NULL)
    {
        cout << "Child successfully received data from STARTUPINFO:\n";
        cout << ">> " << si.lpTitle << "\n";
    }
    else
    {
        cout << "Child: No message found in lpTitle.\n";
    }

    system("pause");
    return 0;
}