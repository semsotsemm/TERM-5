#include <iostream>
#include <windows.h>
#include <cstdlib>
#include <cstring>
#include <vector>
#include <string>
#include <ctime>
#include <iomanip>


using namespace std;


SYNCHRONIZATION_BARRIER stage_barrier;

int thread_count = 0;
int matrix_count = 0;
int matrix_size = 0;

vector<vector<int>> matrices;
vector<LONG64> result_matrix;
LONG64 total_elements_sum = 0;


struct ThreadData
{
	int thread_id;
	int start_matrix_index;
	int end_matrix_index;
};


void PrintLine(char symbol = '-', string line_name = "-", int length = 60)
{
	for (int i = 0; i < length; i++)
	{
		if (i == int(length / 2) && line_name != "-")
		{
			cout << line_name;
		}
		cout << symbol;
	}
	cout << '\n';
}


bool GetStartupInformation(int argc, char* argv[])
{
	if ((argc == 2 && strcmp(argv[1], "--help") == 0) || argc != 4)
	{
		cout << "Использование: Lab-06-bar.exe [Кол-во потоков] [Кол-во матриц] [Размер матриц]\n"
			<< "Пример: Lab-06-bar.exe 4 10 5\n";
		return false;
	}

	try
	{
		thread_count = stoi(argv[1]);
		matrix_count = stoi(argv[2]);
		matrix_size = stoi(argv[3]);
	}
	catch (const exception&)
	{
		cerr << "Ошибка: все аргументы должны быть целыми числами.\n";
		return false;
	}

	if (thread_count <= 0 || matrix_count <= 0 || matrix_size <= 0)
	{
		cerr << "Ошибка: все параметры должны быть больше 0.\n";
		return false;
	}

	return true;
}


void GenerateMatrices()
{
	srand(static_cast<unsigned int>(time(0)));

	int elements_in_matrix = matrix_size * matrix_size;
	matrices.assign(matrix_count, vector<int>(elements_in_matrix, 0));
	result_matrix.assign(elements_in_matrix, 0);

	for (int m = 0; m < matrix_count; m++)
	{
		for (int i = 0; i < elements_in_matrix; i++)
		{
			matrices[m][i] = -2000 + rand() % 4001;
		}
	}
}


void PrintResult()
{
	PrintLine('=', " Итоговая матрица ", 40);
	for (int i = 0; i < matrix_size; i++)
	{
		for (int j = 0; j < matrix_size; j++)
		{
			cout << setw(8) << result_matrix[i * matrix_size + j] << ' ';
		}
		cout << '\n';
	}
	PrintLine('=', "-", 58);
	cout << "Общая сумма всех элементов матриц: " << total_elements_sum << '\n';
}


DWORD WINAPI WorkerThread(LPVOID thread_parameters)
{
	ThreadData* data = static_cast<ThreadData*>(thread_parameters);
	int elements_in_matrix = matrix_size * matrix_size;

	// Этап 1: расчёт частичных сумм каждым потоком
	vector<LONG64> partial_matrix_sum(elements_in_matrix, 0);
	LONG64 partial_elements_sum = 0;

	for (int m = data->start_matrix_index; m < data->end_matrix_index; m++)
	{
		for (int i = 0; i < elements_in_matrix; i++)
		{
			partial_matrix_sum[i] += matrices[m][i];
			partial_elements_sum += matrices[m][i];
		}
	}

	// Барьер после этапа 1
	EnterSynchronizationBarrier(&stage_barrier, 0);

	// Этап 2: сведение всех частичных сумм в одну средствами атомарных операций
	for (int i = 0; i < elements_in_matrix; i++)
	{
		if (partial_matrix_sum[i] != 0)
		{
			InterlockedExchangeAdd64(&result_matrix[i], partial_matrix_sum[i]);
		}
	}
	InterlockedExchangeAdd64(&total_elements_sum, partial_elements_sum);

	// Барьер после этапа 2 (вернёт TRUE ровно для одного потока)
	if (EnterSynchronizationBarrier(&stage_barrier, 0))
	{
		// Этап 3: печать результата на консоль
		PrintResult();
	}

	return 0;
}


int main(int argc, char* argv[])
{
	setlocale(LC_CTYPE, "Russian");

	if (!GetStartupInformation(argc, argv))
	{
		return 1;
	}

	GenerateMatrices();

	if (!InitializeSynchronizationBarrier(&stage_barrier, thread_count, -1))
	{
		cerr << "Ошибка инициализации барьера.\n";
		return 1;
	}

	vector<ThreadData> threads_data(thread_count);
	vector<HANDLE> thread_handles(thread_count, NULL);

	int base_count = matrix_count / thread_count;
	int remainder = matrix_count % thread_count;
	int current_index = 0;

	for (int i = 0; i < thread_count; i++)
	{
		int count_for_thread = base_count + (i < remainder ? 1 : 0);
		threads_data[i].thread_id = i;
		threads_data[i].start_matrix_index = current_index;
		threads_data[i].end_matrix_index = current_index + count_for_thread;
		current_index += count_for_thread;

		thread_handles[i] = CreateThread(NULL, 0, WorkerThread, &threads_data[i], 0, NULL);
		if (thread_handles[i] == NULL)
		{
			cerr << "Ошибка создания потока " << i << '\n';
			DeleteSynchronizationBarrier(&stage_barrier);
			return 1;
		}
	}

	for (int i = 0; i < thread_count; i++)
	{
		WaitForSingleObject(thread_handles[i], INFINITE);
		CloseHandle(thread_handles[i]);
	}

	DeleteSynchronizationBarrier(&stage_barrier);
	return 0;
}