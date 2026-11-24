#include <iostream>
#include <iomanip>
#include <windows.h>


using namespace std;


int main()
{
	SYSTEMTIME local_time, system_time;
	bool is_current_month, is_current_year;
    int total_minutes;
    
	GetLocalTime(&local_time);
	GetSystemTime(&system_time);
    
    total_minutes = (local_time.wHour * 60 + local_time.wMinute) - (system_time.wHour * 60 + system_time.wMinute);

    is_current_year = local_time.wYear == system_time.wYear;
    is_current_month = local_time.wMonth == system_time.wMonth;

    // Решаем проблему разных дней (23:00 > 01:00, хотя наоборот должно быть, потому-что день следующий)
    if (local_time.wYear > system_time.wYear || (is_current_year  && local_time.wMonth > system_time.wMonth) || (is_current_year && is_current_month  && local_time.wDay > system_time.wDay))
    {
        total_minutes += 24 * 60; 
    }
    else if (local_time.wYear < system_time.wYear || (is_current_year && local_time.wMonth < system_time.wMonth) || (is_current_year && is_current_month && local_time.wDay < system_time.wDay))
    {
        total_minutes -= 24 * 60; 
    }

    char sign = (total_minutes >= 0) ? '+' : '-';
    // abs из c  :(
    int abs_minutes = (total_minutes >= 0) ? total_minutes : -total_minutes;
    int hour_offset = abs_minutes / 60;
    int min_offset = abs_minutes % 60;

    cout << setfill('0') << setw(4) << local_time.wYear << '-' << setw(2) << local_time.wMonth << '-' << setw(2) << local_time.wDay << "-T-" << setw(2) << local_time.wHour << ':' << setw(2) << local_time.wMinute << ':' << setw(2) << local_time.wSecond << sign << setw(2) << hour_offset << ':' << setw(2) << min_offset << '\n';
    return 0;
}  