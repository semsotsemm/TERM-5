#include <iostream>
#include <windows.h>


using namespace std;


void PrintLine(char symbol = '-', int line_length = 50)
{
    cout << string(line_length, symbol) + '\n';
}


void GetPriorityValues(const DWORD& priority_class, const int& thread_priority, string& value_priority_class, string& value_thread_priority)
{
    switch (priority_class)
    {
        case IDLE_PRIORITY_CLASS:
        {
            value_priority_class = "IDLE_PRIORITY_CLASS";
            break;
        }
        case BELOW_NORMAL_PRIORITY_CLASS:
        {
            value_priority_class = "BELOW_NORMAL_PRIORITY_CLASS";
            break;
        }
        case NORMAL_PRIORITY_CLASS:
        {
            value_priority_class = "NORMAL_PRIORITY_CLASS";
            break;
        }
        case ABOVE_NORMAL_PRIORITY_CLASS:
        {
            value_priority_class = "ABOVE_NORMAL_PRIORITY_CLASS";
            break;
        }
        case HIGH_PRIORITY_CLASS:
        {
            value_priority_class = "HIGH_PRIORITY_CLASS";
            break;
        }
        case REALTIME_PRIORITY_CLASS:
        {
            value_priority_class = "REALTIME_PRIORITY_CLASS";
            break;
        }
        default:
        {
            value_priority_class = "Not known";
            break;
        }
    }

    switch (thread_priority)
    {
        case THREAD_PRIORITY_HIGHEST:
        {
            value_thread_priority = "THREAD_PRIORITY_HIGHEST";
            break;
        }
        case THREAD_PRIORITY_ABOVE_NORMAL:
        {
            value_thread_priority = "THREAD_PRIORITY_ABOVE_NORMAL";
            break;
        }
        case THREAD_PRIORITY_NORMAL:
        {
            value_thread_priority = "THREAD_PRIORITY_NORMAL";
            break;
        }
        case THREAD_PRIORITY_LOWEST:
        {
            value_thread_priority = "THREAD_PRIORITY_LOWEST";
            break;
        }
        case THREAD_PRIORITY_BELOW_NORMAL:
        {
            value_thread_priority = "THREAD_PRIORITY_BELOW_NORMAL";
            break;
        }
        default:
        {
            value_thread_priority = "Not known";
            break;
        }
    }

}


int main()
{
    HANDLE h_process = GetCurrentProcess();
    HANDLE h_thread = GetCurrentThread();

    int thread_priority = GetThreadPriority(h_thread);
    DWORD priority_class = GetPriorityClass(h_process);
    
    string value_priority_class, value_thread_priority;
    
    if (priority_class == 0 || thread_priority == THREAD_PRIORITY_ERROR_RETURN)
    {
        cerr << GetLastError();
        return 1;
    }
    GetPriorityValues(priority_class, thread_priority, value_priority_class, value_thread_priority);
   
    PrintLine();
    cout << "PID: " << GetCurrentProcessId() << '\n';
    cout << "TID: " << GetCurrentThreadId() << '\n';
    cout << "Process priority class: " << value_priority_class << '\n';
    cout << "Thread priority: " << value_thread_priority << '\n';
    PrintLine();
}