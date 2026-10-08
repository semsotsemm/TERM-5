#include <iostream>
#include <windows.h>

using namespace std;

int main()
{
    STARTUPINFOA startup_info;
    PROCESS_INFORMATION process_info;

    ZeroMemory(&startup_info, sizeof(startup_info));
    startup_info.cb = sizeof(startup_info);

    char secret_message[] = "Hello, my dear child!";
    startup_info.lpTitle = secret_message;

    char child_app[] = "Lab-03-child.exe";

    cout << "Parent: Starting child process in a new window...\n";

    if (CreateProcessA(NULL, child_app, NULL, NULL, FALSE, CREATE_NEW_CONSOLE, NULL, NULL, &startup_info, &process_info))
    {
        WaitForSingleObject(process_info.hProcess, INFINITE);
        CloseHandle(process_info.hThread);
        CloseHandle(process_info.hProcess);
    }
    else
    {
        cout << "Error creating process. Code: " << GetLastError() << "\n";
    }

    return 0;
}