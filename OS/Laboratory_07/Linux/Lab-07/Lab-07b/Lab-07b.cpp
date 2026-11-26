#include <iostream>
#include <ctime>

using namespace std;

int main()
{
    setlocale(LC_CTYPE, "");

    timespec start_cpu_time, current_cpu_time;
    timespec start_real_time, end_real_time;

    clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &start_cpu_time);
    clock_gettime(CLOCK_MONOTONIC, &start_real_time);

    short last_printed_second = 0;
    unsigned long long iteration = 0;
    int seconds_count = 0;

    while (true)
    {
        iteration++;
        clock_gettime(CLOCK_PROCESS_CPUTIME_ID, ¤t_cpu_time);
        seconds_count = (int)((current_cpu_time.tv_sec - start_cpu_time.tv_sec) + (current_cpu_time.tv_nsec - start_cpu_time.tv_nsec) / 1e9);

        if (seconds_count % 5 == 0 && seconds_count != last_printed_second)
        {
            last_printed_second = seconds_count;
            cout << "Количество итераций за " << seconds_count << " секунд: " << iteration << '\n';
            if (seconds_count >= 15)
            {
                break;
            }
        }
    }

    clock_gettime(CLOCK_MONOTONIC, &end_real_time);
    double real_seconds = (end_real_time.tv_sec - start_real_time.tv_sec) + (end_real_time.tv_nsec - start_real_time.tv_nsec) / 1e9;
    cout << "Прошло реального времени: " << real_seconds << " секунд\n";

    return 0;
}