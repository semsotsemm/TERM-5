#include <iostream>
#include <windows.h>


using namespace std;


int main()
{
	setlocale(LC_CTYPE, "Russian");


	ULONGLONG iteration = 0;
	int pass_seconds = 0;


	HANDLE waitable_timer = CreateWaitableTimer(NULL, FALSE, nullptr); // без., ручной сброс, имя в системе
	LARGE_INTEGER first_activation;
	first_activation.QuadPart = -30000000LL; // Относительное время, нс.
	SetWaitableTimer(waitable_timer, &first_activation, 3000, nullptr, nullptr, FALSE); // Обратный возвр., аргументы, будить устройство?


	while (true)
	{
		iteration++;
		if (WaitForSingleObject(waitable_timer, 0) == WAIT_OBJECT_0)
		{
			pass_seconds += 3;
			cout << "Количество итераций за " << pass_seconds << " секунд: " << iteration << '\n';
			if (pass_seconds == 15)
			{
				break;
			}
		}
	}


	CloseHandle(waitable_timer);
	return 0;
}