#include <iostream>
#include <windows.h>
#include <string>


using namespace std;


DWORD GetPriorityClassFromNumber(int p) {
    switch (p) 
    {
        case 1:
        {
            return IDLE_PRIORITY_CLASS;
        }
        case 2:
        {
            return BELOW_NORMAL_PRIORITY_CLASS;
        }
        case 3:
        {
            return NORMAL_PRIORITY_CLASS; 
        }
        case 4:
        {
            return ABOVE_NORMAL_PRIORITY_CLASS;
        }
        case 5:
        {
            return HIGH_PRIORITY_CLASS;  
        }
        case 6:
        {
            return REALTIME_PRIORITY_CLASS;
        }
        default:
        {
            return NORMAL_PRIORITY_CLASS;
        }
    }
}


int main(int argc, char* argv[])
{
    if (argc != 4) 
    {
        cerr << "Usage: Lab-05b.exe <Mask_P1> <Priority_P2> <Priority_P3>\n";
        cerr << "Priorities: 2 = Below Normal, 3 = Normal, 5 = High\n";
        return 1;
    }

    DWORD_PTR p1_mask = stoull(argv[1]);
    int p2_num = stoi(argv[2]);
    int p3_num = stoi(argv[3]);

    DWORD p2_class = GetPriorityClassFromNumber(p2_num);
    DWORD p3_class = GetPriorityClassFromNumber(p3_num);

    cout << "--- Start parameters ---\n";
    cout << "P1 (Mask): " << p1_mask << '\n';
    cout << "P2 (Process priority 1): " << p2_num << '\n';
    cout << "P3 (Process priority 2): " << p3_num << '\n';
    cout << "-------------------------\n";

    HANDLE hCurrentProcess = GetCurrentProcess();
    if (!SetProcessAffinityMask(hCurrentProcess, p1_mask)) 
    {
        cerr << GetLastError() << '\n';
        return 1;
    }

    STARTUPINFOA si1, si2;
    PROCESS_INFORMATION pi1, pi2;

    ZeroMemory(&si1, sizeof(si1));
    si1.cb = sizeof(si1);
    ZeroMemory(&pi1, sizeof(pi1));

    ZeroMemory(&si2, sizeof(si2));
    si2.cb = sizeof(si2);
    ZeroMemory(&pi2, sizeof(pi2));

    char appName[] = "Lab-05x.exe";

    if (!CreateProcessA(NULL, appName, NULL, NULL, FALSE, CREATE_NEW_CONSOLE | p2_class, NULL, NULL, &si1, &pi1))
    {
        cerr << GetLastError() << '\n';
        return 1;
    }

    if (!CreateProcessA(NULL, appName, NULL, NULL, FALSE, CREATE_NEW_CONSOLE | p3_class, NULL, NULL, &si2, &pi2))
    {
        cerr << GetLastError() << '\n';
        return 1;
    }

    cout << "Child process started.\n";

    CloseHandle(pi1.hProcess);
    CloseHandle(pi1.hThread);
    CloseHandle(pi2.hProcess);
    CloseHandle(pi2.hThread);
    return 0;
}