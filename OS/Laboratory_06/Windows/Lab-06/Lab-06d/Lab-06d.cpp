#include <iostream>
#include <windows.h>


using namespace std;


string USERNAME;
HANDLE hEvent;


void ProcessCycle(string process_name)
{
	if (process_name != "main")
	{
		WaitForSingleObject(hEvent, INFINITE);
	}
	for (int i = 1; i < 1001; i++)
	{
		if (process_name == "main")
		{
			if (i == 16)
			{
				SetEvent(hEvent);
			}
		}
		cout << "Процесс: " << process_name << ". Поток: main. Итерация: " << i << ". Символ username: " << USERNAME[i % USERNAME.length()] << '\n';
		Sleep(100);
	}
}


int main(int argc, char* argv[])
{
	setlocale(LC_CTYPE, "Russian");
	char first_command_start[] = "Lab-06d.exe A";
	char second_command_start[] = "Lab-06d.exe B";
	bool is_parent = false;
	string process_name;
	hEvent = CreateEventA(NULL, TRUE, 0, "Lab-06d-event");

	if (argc == 1)
	{
		is_parent = true;
		process_name = "main";
	}

	char* env_value = nullptr;
	size_t len = 0;
	if (_dupenv_s(&env_value, &len, "USERNAME") == 0 && env_value != nullptr)
	{
		USERNAME = env_value;
		free(env_value);
	}
	else
	{
		cerr << "Ошибка получения имени пользователя";
		return 1;
	}


	if (is_parent)
	{
		STARTUPINFOA startup_info1, startup_info2;
		PROCESS_INFORMATION process_info1, process_info2;

		ZeroMemory(&startup_info1, sizeof(startup_info1));
		ZeroMemory(&startup_info2, sizeof(startup_info2));
		startup_info1.cb = sizeof(startup_info1);
		startup_info2.cb = sizeof(startup_info2);
		ZeroMemory(&process_info1, sizeof(process_info1));
		ZeroMemory(&process_info2, sizeof(process_info2));

		if (!CreateProcessA(NULL, first_command_start, NULL, NULL, FALSE, CREATE_NEW_CONSOLE, NULL, NULL, &startup_info1, &process_info1))
		{
			cerr << "Ошибка при создании процесса.";
			return 1;
		}
		if (!CreateProcessA(NULL, second_command_start, NULL, NULL, FALSE, CREATE_NEW_CONSOLE, NULL, NULL, &startup_info2, &process_info2))
		{
			cerr << "Ошибка при создании процесса.";
			return 1;
		}

		ProcessCycle(process_name);

		WaitForSingleObject(process_info1.hProcess, INFINITE);
		WaitForSingleObject(process_info2.hProcess, INFINITE);

		CloseHandle(process_info1.hProcess);
		CloseHandle(process_info2.hProcess);
		CloseHandle(process_info1.hThread);
		CloseHandle(process_info2.hThread);
	}
	else
	{
		process_name = argv[1];
		ProcessCycle(process_name);
	}

	CloseHandle(hEvent);
	return 0;
}