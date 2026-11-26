#include <iostream>
#include <iomanip>
#include <ctime>

using namespace std;

int main()
{
    time_t now = time(nullptr);
    tm local_time, system_time;
    bool is_current_month, is_current_year;
    int total_minutes;

    localtime_r(&now, &local_time);
    gmtime_r(&now, &system_time);

    total_minutes = (local_time.tm_hour * 60 + local_time.tm_min) - (system_time.tm_hour * 60 + system_time.tm_min);

    is_current_year = local_time.tm_year == system_time.tm_year;
    is_current_month = local_time.tm_mon == system_time.tm_mon;

    if (local_time.tm_year > system_time.tm_year || (is_current_year && local_time.tm_mon > system_time.tm_mon) || (is_current_year && is_current_month && local_time.tm_mday > system_time.tm_mday))
    {
        total_minutes += 24 * 60;
    }
    else if (local_time.tm_year < system_time.tm_year || (is_current_year && local_time.tm_mon < system_time.tm_mon) || (is_current_year && is_current_month && local_time.tm_mday < system_time.tm_mday))
    {
        total_minutes -= 24 * 60;
    }

    char sign = (total_minutes >= 0) ? '+' : '-';

    int abs_minutes = (total_minutes >= 0) ? total_minutes : -total_minutes;
    int hour_offset = abs_minutes / 60;
    int min_offset = abs_minutes % 60;

    cout << setfill('0') << setw(4) << (local_time.tm_year + 1900) << '-' << setw(2) << (local_time.tm_mon + 1) << '-' << setw(2) << local_time.tm_mday << "-T-" << setw(2) << local_time.tm_hour << ':' << setw(2) << local_time.tm_min << ':' << setw(2) << local_time.tm_sec << sign << setw(2) << hour_offset << ':' << setw(2) << min_offset << '\n';
    return 0;
}