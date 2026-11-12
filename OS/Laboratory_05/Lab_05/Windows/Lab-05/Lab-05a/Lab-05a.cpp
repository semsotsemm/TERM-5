#include <iostream>
#include <windows.h>
#include <bitset>

using namespace std;

void GetPriorityValues(const DWORD& priority_class, const int& thread_priority, string& value_priority_class)
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
    HANDLE h_process = GetCurrentProcess();
    HANDLE h_thread = GetCurrentThread();

    int thread_priority = GetThreadPriority(h_thread);
    DWORD priority_class = GetPriorityClass(h_process);

    if (priority_class == 0 || thread_priority == THREAD_PRIORITY_ERROR_RETURN)
    {
        cerr << GetLastError();
        return 1;
    }

    string value_priority_class;
    GetPriorityValues(priority_class, thread_priority, value_priority_class);

    DWORD currentProcessor = GetCurrentProcessorNumber();

    DWORD_PTR processAffinityMask;
    DWORD_PTR systemAffinityMask;

    if (!GetProcessAffinityMask(h_process, &processAffinityMask, &systemAffinityMask))
    {
        cerr << GetLastError() << '\n';
        return 1;
    }

    int availableProcessors = 0;
    DWORD_PTR maskCopy = processAffinityMask;
    while (maskCopy)
    {
        availableProcessors += (maskCopy & 1);
        maskCopy >>= 1;
    }

    cout << "ITER=" << 0
        << " PID=" << GetCurrentProcessId()
        << " TID=" << GetCurrentThreadId()
        << " CLASS=" << value_priority_class
        << " CLASS_CODE=" << priority_class
        << " THREAD_REL=" << thread_priority
        << " BASE=" << "8"
        << " CPU=" << currentProcessor << '\n';

    cout << "PROCESS_MASK_BIN=" << bitset<sizeof(DWORD_PTR) * 8>(processAffinityMask) << '\n';
    cout << "MASK_DEC=" << processAffinityMask << '\n';
    cout << "CPU_COUNT=" << availableProcessors << '\n';
    cout << "ONE_CPU_MASK_DEC=" << 1 << '\n';
    cout << "SYSTEM_MASK_BIN=" << bitset<sizeof(DWORD_PTR) * 8>(systemAffinityMask) << '\n';

    return 0;
}