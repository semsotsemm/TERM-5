#include <iostream>
#include <windows.h>
#include <string>
#include <vector>


using namespace std;


vector<int> FindSimpleNumber(HANDLE waitable_timer)
{
	vector<int> simple_numbers;
	int current_number = 3;
	bool is_simple;
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


		if (WaitForSingleObject(waitable_timer, 0) == WAIT_OBJECT_0)
		{
			return simple_numbers;
		}
	}
}


int main(int argc, char* argv[])
{
	setlocale(LC_CTYPE, "Russian");
	int work_seconds = 0;
	ios_base::sync_with_stdio(false);


	if (argc < 2 || (argc == 2 && string(argv[1]) == "--help"))
	{
		cout << "Использование: Lab-07x.exe [время работы в с.]";
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
		cout << "Ошибка: второй параметр должен быть целочисленным, положительным числом";
		return 1;
	}


	HANDLE waitable_timer = CreateWaitableTimer(NULL, FALSE, nullptr);
	LARGE_INTEGER first_action;
	first_action.QuadPart = -10000000LL * work_seconds;

	SetWaitableTimer(waitable_timer, &first_action, 0, NULL, NULL, FALSE);


	ULONGLONG start_time = GetTickCount64();

	vector<int> simple_numbers = FindSimpleNumber(waitable_timer);

	string output = "";
	for (int i = 0; i < simple_numbers.size(); i++)
	{
		output += to_string(i + 1) + ". " + to_string(simple_numbers[i]) + "\t";
	}
	cout << output;
	
	string printed_line(50, '=');
	cout << '\n' << printed_line << '\n';
	cout << "Время работы: " << int((GetTickCount() - start_time) / 1000) << "." << (GetTickCount() - start_time) % 1000 << "c.\n"; 
	cout << "Всего обработано чисел:" << simple_numbers[simple_numbers.size() - 1] << '\n';
	cout << "Найдено простых чисел" << simple_numbers.size() << '\n';
	cout << printed_line << '\n';


	CloseHandle(waitable_timer);
	return 0;
}