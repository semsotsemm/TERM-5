#include <iostream>
#include <windows.h>
#include <timeapi.h>
#include <vector>
#include <string>
#include <unordered_map>
#include <algorithm>

#pragma comment(lib, "winmm.lib")

using namespace std;

void runTest(const string& title)
{
    unordered_map<string, vector<double>> true_sleep_time = {
        { "sleep_1", vector<double>() },
        { "sleep_5", vector<double>() },
        { "sleep_10", vector<double>() },
        { "sleep_15", vector<double>() },
        { "sleep_20", vector<double>() },
    };
    vector<int> checking_sleep_time = { 1, 5, 10, 15, 20 };
    LARGE_INTEGER start_time, freq, end_time;

    QueryPerformanceFrequency(&freq);

    for (int i = 0; i < 1000; i++)
    {
        for (int j = 0; j < 5; j++)
        {
            QueryPerformanceCounter(&start_time);
            Sleep(checking_sleep_time[j]);
            QueryPerformanceCounter(&end_time);
            true_sleep_time["sleep_" + to_string(checking_sleep_time[j])].push_back(
                (double)(end_time.QuadPart - start_time.QuadPart) * 1000 / freq.QuadPart
            );
        }
    }

    cout << "\n--- " << title << " ---\n";
    cout << "Sleep\t|Минимум\t|Максимум\t|Среднее\t|Медиана\n";

    for (int i = 0; i < 5; i++)
    {
        string key = "sleep_" + to_string(checking_sleep_time[i]);
        vector<double>& times = true_sleep_time[key];

        sort(times.begin(), times.end());

        double sum = 0;
        for (int j = 0; j < 1000; j++) {
            sum += times[j];
        }

        double min_val = times.front();
        double max_val = times.back();
        double average = sum / 1000.0;
        double median = (times[499] + times[500]) / 2.0;

        cout << checking_sleep_time[i] << " ms\t|"
            << min_val << "\t|"
            << max_val << "\t|"
            << average << "\t|"
            << median << "\n";
    }
}

int main()
{
    setlocale(LC_CTYPE, "Russian");

    runTest("Без timeBeginPeriod");

    timeBeginPeriod(1);
    runTest("С timeBeginPeriod(1)");
    timeEndPeriod(1);

    return 0;
}