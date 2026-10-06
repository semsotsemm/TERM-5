#include <iostream>
#include <windows.h>
#include <tlhelp32.h> 
#include <iomanip>   

using namespace std;

int main()
{
    HANDLE hProcessSnap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);

    if (hProcessSnap == INVALID_HANDLE_VALUE)
    {
        cout << "Error: Failed to create snapshot. Error code: " << GetLastError() << "\n";
        return 1;
    }

    PROCESSENTRY32 pe32;
    pe32.dwSize = sizeof(PROCESSENTRY32);

    if (!Process32First(hProcessSnap, &pe32))
    {
        cout << "Error: Process32First failed. Error code: " << GetLastError() << "\n";
        CloseHandle(hProcessSnap);
        return 1;
    }

    cout << left << setw(10) << "PID" << setw(15) << "Parent PID" << "Process Name" << "\n";
    cout << string(50, '-') << "\n";

    do
    {
        cout << left << setw(10) << pe32.th32ProcessID << setw(15) << pe32.th32ParentProcessID << pe32.szExeFile << "\n";

    } while (Process32Next(hProcessSnap, &pe32)); 

    CloseHandle(hProcessSnap);

    system("pause");
    return 0;
}