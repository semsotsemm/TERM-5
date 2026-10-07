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

    PROCESSENTRY32 process_info;
    process_info.dwSize = sizeof(PROCESSENTRY32);

    if (!Process32First(hProcessSnap, &process_info))
    {
        cout << "Error: Process32First failed. Error code: " << GetLastError() << "\n";
        CloseHandle(hProcessSnap);
        return 1;
    }

    cout << left << setw(10) << "PID" << setw(15) << "Parent PID" << "Process Name" << "\n";
    cout << string(50, '-') << "\n";

    do
    {
        wcout << left << setw(10) << process_info.th32ProcessID << setw(15) << process_info.th32ParentProcessID << process_info.szExeFile << L"\n";

    } while (Process32Next(hProcessSnap, &process_info)); 

    CloseHandle(hProcessSnap);
    cout << "\n";

    system("pause");
    return 0;
}