#include <iostream>
#include <string>
#include <vector>
#include <ctime>
#include <sys/timerfd.h>
#include <unistd.h>
#include <cstdint>

using namespace std;

vector<int> FindSimpleNumber(int waitable_timer)
{
    vector<int> simple_numbers;
    int current_number = 3;
    bool is_simple;
    uint64_t expirations;
    simple_numbers.push_back(2);

    while (true)
    {
        is_simple = true;
        for (int i = 3; i * i <= current_number; i += 2)
        {
            if (current_number % i == 0)
            {
                is_simple = false;
                break;
            }
        }
        if (is_simple == true)
        {
            simple_numbers.push_back(current_number);
        }
        current_number += 2;

        if (read(waitable_timer, &expirations, sizeof(expirations)) > 0)
        {
            return simple_numbers;
        }
    }
}

int main(int argc, char* argv[])
{
    setlocale(LC_CTYPE, "");
    int work_seconds = 0;
    ios_base::sync_with_stdio(false);

    if (argc < 2 || (argc == 2 && string(argv[1]) == "--help"))
    {
        cout << "Использование: ./Lab-07x [время работы в с.]\n";
        return 0;
    }
    try
    {
        work_seconds = stoi(argv[1]);
        if (work_seconds <= 0)
        {
            throw exception();
        }
    }
    catch (const exception&)
    {
        cout << "Ошибка: второй параметр должен быть целочисленным, положительным числом\n";
        return 1;
    }

    int waitable_timer = timerfd_create(CLOCK_MONOTONIC, TFD_NONBLOCK);
    itimerspec first_action = {};
    first_action.it_value.tv_sec = work_seconds;
    timerfd_settime(waitable_timer, 0, &first_action, nullptr);

    timespec start_time, end_time;
    clock_gettime(CLOCK_MONOTONIC, &start_time);

    vector<int> simple_numbers = FindSimpleNumber(waitable_timer);

    clock_gettime(CLOCK_MONOTONIC, &end_time);
    long long elapsed_ms = (end_time.tv_sec - start_time.tv_sec) * 1000LL + (end_time.tv_nsec - start_time.tv_nsec) / 1000000LL;

    string output = "";
    for (size_t i = 0; i < simple_numbers.size(); i++)
    {
        output += to_string(i + 1) + ". " + to_string(simple_numbers[i]) + "\t";
    }
    cout << output;

    string printed_line(50, '=');
    cout << '\n' << printed_line << '\n';
    cout << "Время работы: " << (elapsed_ms / 1000) << "." << (elapsed_ms % 1000) << "c.\n";
    cout << "Всего обработано чисел: " << simple_numbers[simple_numbers.size() - 1] << '\n';
    cout << "Найдено простых чисел: " << simple_numbers.size() << '\n';
    cout << printed_line << '\n';

    close(waitable_timer);
    return 0;
}