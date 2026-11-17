	#include <iostream>
	#include <windows.h>
	#include <cstdlib>


	using namespace std;


	string USERNAME;
	CRITICAL_SECTION thread_cs;


	struct ThreadData 
	{
		string thread_name;
		int iteration_count;
	};


	DWORD WINAPI ThreadFunction(LPVOID thread_parameters)
	{
		ThreadData* data = static_cast<ThreadData*>(thread_parameters);

		for (int i = 1; i < data->iteration_count + 1; i++)
		{
			if (i == 30)
			{
				EnterCriticalSection(&thread_cs);
			}
			cout << "Поток: " << data->thread_name << ". Итерация: " << i << ". Символ username: " << USERNAME[i % USERNAME.length()] << '\n';
			Sleep(100);
			if (i == 60)
			{
				LeaveCriticalSection(&thread_cs);
			}
		}
		return 0;
	}


	int main()
	{
		setlocale(LC_CTYPE, "Russian");
		InitializeCriticalSection(&thread_cs);

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

		ThreadData process_a_data = { "A", 100 };
		ThreadData process_b_data = { "B", 100 };
		ThreadData process_main_data = { "main", 100 };

		HANDLE hThreadA = CreateThread(NULL, 0, ThreadFunction, &process_a_data, 0, NULL);
		HANDLE hThreadB = CreateThread(NULL, 0, ThreadFunction, &process_b_data, 0, NULL);
		if (hThreadA == NULL || hThreadB == NULL)
		{
			cerr << "Ошибка создания потока одного из потоков\n";
			return 1;
		}

		ThreadFunction(&process_main_data);

		HANDLE threads[2] = { hThreadA, hThreadB };
		WaitForMultipleObjects(2, threads, TRUE, INFINITE);

		CloseHandle(hThreadA);
		CloseHandle(hThreadB);
		DeleteCriticalSection(&thread_cs);
		return 0;
	}