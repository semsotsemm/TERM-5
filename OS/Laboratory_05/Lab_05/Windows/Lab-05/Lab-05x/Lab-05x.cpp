#include <iostream>
#include <windows.h>
#include <ctime>

using namespace std;

void GetPriorityValues(const DWORD& priority_class, string& value_priority_class)
{
    switch (priority_class)
    {
    case IDLE_PRIORITY_CLASS:
        value_priority_class = "Idle";
        break;
    case BELOW_NORMAL_PRIORITY_CLASS:
        value_priority_class = "Below Normal";
        break;
    case NORMAL_PRIORITY_CLASS:
        value_priority_class = "Normal";
        break;
    case ABOVE_NORMAL_PRIORITY_CLASS:
        value_priority_class = "Above Normal";
        break;
    case HIGH_PRIORITY_CLASS:
        value_priority_class = "High";
        break;
    case REALTIME_PRIORITY_CLASS:
        value_priority_class = "Realtime";
        break;
    default:
        value_priority_class = "Unknown";
        break;
    }
}

int main()
{
    clock_t start_time = clock();

    HANDLE h_process = GetCurrentProcess();
    HANDLE h_thread = GetCurrentThread();
    string value_priority_class;

    for (int i = 1; i <= 1000000; i++)
    {
        if (i % 1000 == 0)
        {
            Sleep(200);

            int thread_priority = GetThreadPriority(h_thread);
            DWORD priority_class = GetPriorityClass(h_process);
            DWORD currentProcessor = GetCurrentProcessorNumber();

            GetPriorityValues(priority_class, value_priority_class);

            cout << "ITER=" << i
                << " PID=" << GetCurrentProcessId()
                << " TID=" << GetCurrentThreadId()
                << " CLASS=" << value_priority_class
                << " CLASS_CODE=" << priority_class
                << " THREAD_REL=" << thread_priority
                << " BASE=" << "8"
                << " CPU=" << currentProcessor << '\n';
        }
    }

    clock_t end_time = clock();
    double time_spent = (double)(end_time - start_time) / CLOCKS_PER_SEC;

    cout << "\nPast time: " << time_spent << " seconds.\n";

    return 0;
}