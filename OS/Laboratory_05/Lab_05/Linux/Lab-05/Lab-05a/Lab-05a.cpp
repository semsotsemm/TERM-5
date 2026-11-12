#include <iostream>
#include <string>
#include <bitset>
#include <unistd.h>
#include <sys/syscall.h>
#include <pthread.h>
#include <sched.h>
#include <sys/resource.h>


using namespace std;


void PrintLine(char symbol = '-', int line_length = 50)
{
    cout << string(line_length, symbol) + '\n';
}


int main()
{
    pid_t pid = getpid();
    pid_t tid = syscall(SYS_gettid);

    int nice_value = getpriority(PRIO_PROCESS, 0);

    int policy;
    struct sched_param param;
    pthread_getschedparam(pthread_self(), &policy, ¶m);

    string policy_name;
    switch (policy) {
        case SCHED_OTHER:
        {
            policy_name = "SCHED_OTHER (Normal)";
            break;
        }
        case SCHED_FIFO: 
        {
            policy_name = "SCHED_FIFO (Realtime)"; 
            break;
        }
        case SCHED_RR:   
        {
            policy_name = "SCHED_RR (Realtime)"; 
            break;
        }
        case SCHED_BATCH: 
        {
            policy_name = "SCHED_BATCH"; 
            break;
        }
        case SCHED_IDLE: 
        {
            policy_name = "SCHED_IDLE"; 
            break;
        }
        default:         
        {
            policy_name = "Unknown"; 
            break;
        }
    }

    cpu_set_t cpuset;
    CPU_ZERO(&cpuset);

    if (pthread_getaffinity_np(pthread_self(), sizeof(cpu_set_t), &cpuset) != 0) 
    {
        cerr << "Get affinity error\n";
        return 1;
    }

    int num_cores = sysconf(_SC_NPROCESSORS_ONLN); 
    int available_cpus = CPU_COUNT(&cpuset);      

    string affinity_mask_str = "";
    for (int i = num_cores - 1; i >= 0; i--)
    {
        if (CPU_ISSET(i, &cpuset)) 
        {
            affinity_mask_str += "1";
        }
        else 
        {
            affinity_mask_str += "0";
        }
    }

    int current_cpu = sched_getcpu();

    PrintLine();
    cout << "PID: " << pid << '\n';
    cout << "TID: " << tid << '\n';

    cout << "Класс приоритетов процесса (Nice): " << nice_value << '\n';
    cout << "Класс планирования потока: " << policy_name << '\n';
    cout << "Приоритет потока (Realtime priority): " << param.sched_priority << '\n';

    cout << "Маска родственности процесса: " << affinity_mask_str << '\n';
    cout << "Количество доступных процессоров: " << available_cpus << '\n';
    cout << "Номер текущего процессора: " << current_cpu << '\n';
    PrintLine();

    return 0;
}