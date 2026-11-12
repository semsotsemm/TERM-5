#include <iostream>
#include <windows.h>
#include <string>
#include <ctime>
#include <mutex>

using namespace std;

mutex consoleMutex;

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

DWORD WINAPI ThreadFunction(LPVOID lpParam)
{
    int threadNum = *(int*)lpParam;
    clock_t start_time = clock();
    HANDLE h_process = GetCurrentProcess();
    HANDLE h_thread = GetCurrentThread();
    string value_priority_class;

    for (int i = 1; i <= 1000000; ++i)
    {
        if (i % 1000 == 0)
        {
            Sleep(200);

            int thread_priority = GetThreadPriority(h_thread);
            DWORD priority_class = GetPriorityClass(h_process);
            DWORD currentProcessor = GetCurrentProcessorNumber();

            GetPriorityValues(priority_class, value_priority_class);

            consoleMutex.lock();
            cout << "ITER=" << i
                << " PID=" << GetCurrentProcessId()
                << " TID=" << GetCurrentThreadId()
                << " CLASS=" << value_priority_class
                << " CLASS_CODE=" << priority_class
                << " THREAD_REL=" << thread_priority
                << " BASE=" << "8"
                << " CPU=" << currentProcessor << '\n';
            consoleMutex.unlock();
        }
    }

    clock_t end_time = clock();
    double time_spent = (double)(end_time - start_time) / CLOCKS_PER_SEC;

    consoleMutex.lock();
    cout << "\nThread #" << threadNum << " stop. Time: " << time_spent << " seconds.\n";
    consoleMutex.unlock();

    return 0;
}

DWORD GetProcessPriorityClassFromNumber(int p) {
    switch (p) {
    case 1: return IDLE_PRIORITY_CLASS;
    case 2: return BELOW_NORMAL_PRIORITY_CLASS;
    case 3: return NORMAL_PRIORITY_CLASS;
    case 4: return ABOVE_NORMAL_PRIORITY_CLASS;
    case 5: return HIGH_PRIORITY_CLASS;
    case 6: return REALTIME_PRIORITY_CLASS;
    default: return NORMAL_PRIORITY_CLASS;
    }
}

int GetThreadPriorityFromNumber(int p) {
    switch (p) {
    case 1: return THREAD_PRIORITY_IDLE;
    case 2: return THREAD_PRIORITY_LOWEST;
    case 3: return THREAD_PRIORITY_BELOW_NORMAL;
    case 4: return THREAD_PRIORITY_NORMAL;
    case 5: return THREAD_PRIORITY_ABOVE_NORMAL;
    case 6: return THREAD_PRIORITY_HIGHEST;
    default: return THREAD_PRIORITY_NORMAL;
    }
}

int main(int argc, char* argv[])
{
    if (argc != 5)
    {
        cerr << "Usage: Lab-05c.exe <P1_Mask> <P2_Process Priority> <P3_Thread priority 1> <P4_Thread priority 2>\n";
        return 1;
    }

    DWORD_PTR p1_mask = stoull(argv[1]);
    int p2_num = stoi(argv[2]);
    int p3_num = stoi(argv[3]);
    int p4_num = stoi(argv[4]);

    DWORD p2_class = GetProcessPriorityClassFromNumber(p2_num);
    int p3_priority = GetThreadPriorityFromNumber(p3_num);
    int p4_priority = GetThreadPriorityFromNumber(p4_num);

    HANDLE hCurrentProcess = GetCurrentProcess();

    if (!SetProcessAffinityMask(hCurrentProcess, p1_mask))
    {
        cerr << GetLastError() << '\n';
        return 1;
    }

    if (!SetPriorityClass(hCurrentProcess, p2_class)) {
        cerr << GetLastError() << '\n';
        return 1;
    }

    int id1 = 1, id2 = 2;
    HANDLE hThread1 = CreateThread(NULL, 0, ThreadFunction, &id1, CREATE_SUSPENDED, NULL);
    HANDLE hThread2 = CreateThread(NULL, 0, ThreadFunction, &id2, CREATE_SUSPENDED, NULL);

    if (hThread1 == NULL || hThread2 == NULL) {
        cerr << "Creating thread error.\n";
        return 1;
    }

    SetThreadPriority(hThread1, p3_priority);
    SetThreadPriority(hThread2, p4_priority);

    ResumeThread(hThread1);
    ResumeThread(hThread2);

    HANDLE handles[2] = { hThread1, hThread2 };
    WaitForMultipleObjects(2, handles, TRUE, INFINITE);

    CloseHandle(hThread1);
    CloseHandle(hThread2);

    return 0;
}