#include <iostream>
#include <windows.h>
#include <vector>


using namespace std;


int main()
{
    setlocale(LC_CTYPE, "Russian");
    vector<double> true_sleep_time;
    vector<int> checking_sleep_time = {1, 5, 10, 15, 20};
    LARGE_INTEGER start_time, freq, end_time;

    QueryPerformanceFrequency(&freq);


    for (int i = 0; i < 5; i++)
    {
        QueryPerformanceCounter(&start_time);
        Sleep(checking_sleep_time[i]);
        QueryPerformanceCounter(&end_time);
        true_sleep_time.push_back((double)(end_time.QuadPart - start_time.QuadPart) * 1000 / freq.QuadPart);
    }


    for (int i = 0; i < 5; i++)
    {
        cout << "Sleep(" << checking_sleep_time[i] << ") на самом деле работает " << true_sleep_time[i] << " мс. (должен " << checking_sleep_time[i] << "мс.) погрешность sleep: " << true_sleep_time[i] / checking_sleep_time[i] - 1 << '\n';
    }

}