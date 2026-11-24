#include <iostream>
#include <windows.h>


using namespace std;


int main()
{
	setlocale(LC_CTYPE, "Russian");

	ULONGLONG start_time = GetTickCount64();
	ULONGLONG current_time = GetTickCount64();
	short last_printed_second = 0;

	ULONGLONG iteration = 0;
	int seconds_count = 0;

	while (true)
	{
		iteration++;
		seconds_count = int((current_time - start_time) / 1000);
		if (seconds_count % 5 == 0 && seconds_count != last_printed_second)
		{
			last_printed_second = seconds_count;
			cout << "Количество итераций за " << seconds_count << " секунд: " << iteration << '\n';
			if (seconds_count >= 15)
			{
				break;
			}
		}
		current_time = GetTickCount64();
	}
}