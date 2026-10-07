#include <iostream>
#include <windows.h>
#include <vector>
#include <string>

using namespace std;

int main()
{
    string first_call_string = "Lab-03x.exe";
    char second_call_string[] = "Lab-03x.exe 50";
    string third_call_string = "Lab-03x.exe";
    char third_argument[] = " 50";

    STARTUPINFOA startup_info;
    PROCESS_INFORMATION process_info;
    vector<HANDLE> hProcesses;

    cout << "First process starting...\n";

    ZeroMemory(&startup_info, sizeof(startup_info));
    startup_info.cb = sizeof(startup_info);

    /*
        1. char* на команду для командной строки
        2. Аргументы командой строки
        3. Права доступа дочернего процесса
        4. Настройки безопастности  первичного потока процесса
        5. Наследовать ли открытые файлы, каналы.. от родителя
        6. Флаги создания
        7. Блок переменных окружения
        8. Рабочая папка
        9. Блок для информации о запуске
        10. Блок на стркутуру, куда запишем pid
    */
    if (CreateProcessA(first_call_string.c_str(), NULL, NULL, NULL, FALSE, 0, NULL, NULL, &startup_info, &process_info))
    {
        cout << "Successfull! PID: " << process_info.dwProcessId << "\n";
        hProcesses.push_back(process_info.hProcess);
        CloseHandle(process_info.hThread);
    }
    else
    {
        cout << "Error! Error code: " << GetLastError() << " (Failed to create process).\n";
    }


    cout << "Second process starting...\n";
    ZeroMemory(&startup_info, sizeof(startup_info));
    startup_info.cb = sizeof(startup_info);

    if (CreateProcessA(NULL, second_call_string, NULL, NULL, FALSE, 0, NULL, NULL, &startup_info, &process_info))
    {
        cout << "Successfull! PID: " << process_info.dwProcessId << "\n";
        hProcesses.push_back(process_info.hProcess);
        CloseHandle(process_info.hThread);
    }
    else
    {
        cout << "Error! Error code: " << GetLastError() << " (Failed to create process).\n";
    }

    cout << "Third process starting...\n";
    ZeroMemory(&startup_info, sizeof(startup_info));
    startup_info.cb = sizeof(startup_info);

    if (SetEnvironmentVariableA("ITER_NUM", "60"))
    {
        cout << "Local environment variable ITER_NUM is set to 60.\n";
    }

    if (CreateProcessA(third_call_string.c_str(), NULL, NULL, NULL, FALSE, 0, NULL, NULL, &startup_info, &process_info))
    {
        cout << "Successfull! PID: " << process_info.dwProcessId << "\n";
        hProcesses.push_back(process_info.hProcess);
        CloseHandle(process_info.hThread);
    }
    else
    {
        cout << "Error! Error code: " << GetLastError() << " (Failed to create process).\n";
    }

    if (!hProcesses.empty())
    {
        cout << "\nWait to complete child process...\n";
        WaitForMultipleObjects(hProcesses.size(), hProcesses.data(), TRUE, INFINITE);
        cout << "All process are completed.\n";

        for (HANDLE h : hProcesses)
        {
            CloseHandle(h);
        }
    }

    system("pause");
    return 0;
}