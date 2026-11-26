#include <iostream>
#include <iomanip>
#include <windows.h>
#include <timeapi.h>
#include <intrin.h>

#pragma comment(lib, "winmm.lib")

using namespace std;

ULONGLONG convert_filetime_to_uint64(const FILETIME& file_time)
{
    ULARGE_INTEGER large_int;
    large_int.LowPart = file_time.dwLowDateTime;
    large_int.HighPart = file_time.dwHighDateTime;
    return large_int.QuadPart;
}

double calculate_rdtsc_frequency(const LARGE_INTEGER& performance_frequency)
{
    LARGE_INTEGER start_counter, end_counter;
    QueryPerformanceCounter(&start_counter);
    unsigned __int64 start_ticks = __rdtsc();
    Sleep(100);
    unsigned __int64 end_ticks = __rdtsc();
    QueryPerformanceCounter(&end_counter);

    double elapsed_seconds = (double)(end_counter.QuadPart - start_counter.QuadPart) / performance_frequency.QuadPart;
    return (double)(end_ticks - start_ticks) / elapsed_seconds;
}

int main()
{
    setlocale(LC_CTYPE, "Russian");

    LARGE_INTEGER performance_frequency, start_counter, end_counter;
    QueryPerformanceFrequency(&performance_frequency);

    const int overhead_iterations = 1000000;
    const int slow_step_iterations = 30;
    const int fast_step_iterations = 1000;
    volatile ULONGLONG dummy_accumulator = 0;

    double rdtsc_frequency_hz = calculate_rdtsc_frequency(performance_frequency);

    double tick_count_min_step_ns = 0;
    for (int i = 0; i < slow_step_iterations; i++)
    {
        ULONGLONG first_time = GetTickCount64();
        ULONGLONG second_time = first_time;
        while (second_time == first_time)
        {
            second_time = GetTickCount64();
        }
        double step_ns = (double)(second_time - first_time) * 1000000.0;
        if (tick_count_min_step_ns == 0 || step_ns < tick_count_min_step_ns)
        {
            tick_count_min_step_ns = step_ns;
        }
    }
    QueryPerformanceCounter(&start_counter);
    for (int i = 0; i < overhead_iterations; i++)
    {
        dummy_accumulator += GetTickCount64();
    }
    QueryPerformanceCounter(&end_counter);
    double tick_count_overhead_ns = (double)(end_counter.QuadPart - start_counter.QuadPart) * 1e9 / performance_frequency.QuadPart / overhead_iterations;

    double performance_counter_min_step_ns = 0;
    for (int i = 0; i < fast_step_iterations; i++)
    {
        LARGE_INTEGER first_counter, second_counter;
        QueryPerformanceCounter(&first_counter);
        second_counter = first_counter;
        while (second_counter.QuadPart == first_counter.QuadPart)
        {
            QueryPerformanceCounter(&second_counter);
        }
        double step_ns = (double)(second_counter.QuadPart - first_counter.QuadPart) * 1e9 / performance_frequency.QuadPart;
        if (performance_counter_min_step_ns == 0 || step_ns < performance_counter_min_step_ns)
        {
            performance_counter_min_step_ns = step_ns;
        }
    }
    QueryPerformanceCounter(&start_counter);
    for (int i = 0; i < overhead_iterations; i++)
    {
        LARGE_INTEGER temp_counter;
        QueryPerformanceCounter(&temp_counter);
        dummy_accumulator += temp_counter.QuadPart;
    }
    QueryPerformanceCounter(&end_counter);
    double performance_counter_overhead_ns = (double)(end_counter.QuadPart - start_counter.QuadPart) * 1e9 / performance_frequency.QuadPart / overhead_iterations;

    double system_filetime_min_step_ns = 0;
    for (int i = 0; i < slow_step_iterations; i++)
    {
        FILETIME first_filetime, second_filetime;
        GetSystemTimeAsFileTime(&first_filetime);
        ULONGLONG first_time = convert_filetime_to_uint64(first_filetime);
        ULONGLONG second_time = first_time;
        while (second_time == first_time)
        {
            GetSystemTimeAsFileTime(&second_filetime);
            second_time = convert_filetime_to_uint64(second_filetime);
        }
        double step_ns = (double)(second_time - first_time) * 100.0;
        if (system_filetime_min_step_ns == 0 || step_ns < system_filetime_min_step_ns)
        {
            system_filetime_min_step_ns = step_ns;
        }
    }
    QueryPerformanceCounter(&start_counter);
    for (int i = 0; i < overhead_iterations; i++)
    {
        FILETIME temp_filetime;
        GetSystemTimeAsFileTime(&temp_filetime);
        dummy_accumulator += temp_filetime.dwLowDateTime;
    }
    QueryPerformanceCounter(&end_counter);
    double system_filetime_overhead_ns = (double)(end_counter.QuadPart - start_counter.QuadPart) * 1e9 / performance_frequency.QuadPart / overhead_iterations;

    double precise_filetime_min_step_ns = 0;
    for (int i = 0; i < fast_step_iterations; i++)
    {
        FILETIME first_filetime, second_filetime;
        GetSystemTimePreciseAsFileTime(&first_filetime);
        ULONGLONG first_time = convert_filetime_to_uint64(first_filetime);
        ULONGLONG second_time = first_time;
        while (second_time == first_time)
        {
            GetSystemTimePreciseAsFileTime(&second_filetime);
            second_time = convert_filetime_to_uint64(second_filetime);
        }
        double step_ns = (double)(second_time - first_time) * 100.0;
        if (precise_filetime_min_step_ns == 0 || step_ns < precise_filetime_min_step_ns)
        {
            precise_filetime_min_step_ns = step_ns;
        }
    }
    QueryPerformanceCounter(&start_counter);
    for (int i = 0; i < overhead_iterations; i++)
    {
        FILETIME temp_filetime;
        GetSystemTimePreciseAsFileTime(&temp_filetime);
        dummy_accumulator += temp_filetime.dwLowDateTime;
    }
    QueryPerformanceCounter(&end_counter);
    double precise_filetime_overhead_ns = (double)(end_counter.QuadPart - start_counter.QuadPart) * 1e9 / performance_frequency.QuadPart / overhead_iterations;

    double time_get_time_min_step_ns = 0;
    for (int i = 0; i < slow_step_iterations; i++)
    {
        DWORD first_time = timeGetTime();
        DWORD second_time = first_time;
        while (second_time == first_time)
        {
            second_time = timeGetTime();
        }
        double step_ns = (double)(second_time - first_time) * 1000000.0;
        if (time_get_time_min_step_ns == 0 || step_ns < time_get_time_min_step_ns)
        {
            time_get_time_min_step_ns = step_ns;
        }
    }
    QueryPerformanceCounter(&start_counter);
    for (int i = 0; i < overhead_iterations; i++)
    {
        dummy_accumulator += timeGetTime();
    }
    QueryPerformanceCounter(&end_counter);
    double time_get_time_overhead_ns = (double)(end_counter.QuadPart - start_counter.QuadPart) * 1e9 / performance_frequency.QuadPart / overhead_iterations;

    double rdtsc_min_step_ns = 0;
    for (int i = 0; i < fast_step_iterations; i++)
    {
        unsigned __int64 first_ticks = __rdtsc();
        unsigned __int64 second_ticks = first_ticks;
        while (second_ticks == first_ticks)
        {
            second_ticks = __rdtsc();
        }
        double step_ns = (double)(second_ticks - first_ticks) * 1e9 / rdtsc_frequency_hz;
        if (rdtsc_min_step_ns == 0 || step_ns < rdtsc_min_step_ns)
        {
            rdtsc_min_step_ns = step_ns;
        }
    }
    QueryPerformanceCounter(&start_counter);
    for (int i = 0; i < overhead_iterations; i++)
    {
        dummy_accumulator += __rdtsc();
    }
    QueryPerformanceCounter(&end_counter);
    double rdtsc_overhead_ns = (double)(end_counter.QuadPart - start_counter.QuadPart) * 1e9 / performance_frequency.QuadPart / overhead_iterations;

    cout << fixed << setprecision(2);
    cout << "Источник\t\t\t\t|Мин. шаг (нс)\t\t|Вызов (нс)\n";
    cout << "GetTickCount64\t\t\t\t|" << tick_count_min_step_ns << "\t\t|" << tick_count_overhead_ns << '\n';
    cout << "QueryPerformanceCounter\t\t\t|" << performance_counter_min_step_ns << "\t\t\t|" << performance_counter_overhead_ns << '\n';
    cout << "GetSystemTimeAsFileTime\t\t\t|" << system_filetime_min_step_ns << "\t\t|" << system_filetime_overhead_ns << '\n';
    cout << "GetSystemTimePreciseAsFileTime\t\t|" << precise_filetime_min_step_ns << "\t\t\t|" << precise_filetime_overhead_ns << '\n';
    cout << "timeGetTime\t\t\t\t|" << time_get_time_min_step_ns << "\t\t|" << time_get_time_overhead_ns << '\n';
    cout << "rdtsc\t\t\t\t\t|" << rdtsc_min_step_ns << "\t\t\t|" << rdtsc_overhead_ns << '\n';

    return 0;
}