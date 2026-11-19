#include <iostream>
#include <windows.h>
#include <vector>
#include <string>
#include <iomanip>


using namespace std;


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


void PrintVector(const string& title, const vector<int>& vec)
{
	cout << title << ": ( ";
	for (size_t i = 0; i < vec.size(); i++)
	{
		cout << vec[i] << ' ';
	}
	cout << ")\n";
}


void PrintMatrix(const string& title, const vector<vector<int>>& matrix)
{
	cout << title << ":\n";
	for (size_t i = 0; i < matrix.size(); i++)
	{
		cout << "  P" << i + 1 << ": ";
		for (size_t j = 0; j < matrix[i].size(); j++)
		{
			cout << setw(3) << matrix[i][j] << ' ';
		}
		cout << '\n';
	}
}


bool CanSatisfyRequest(const vector<int>& request_row, const vector<int>& work_vector)
{
	for (size_t j = 0; j < request_row.size(); j++)
	{
		if (request_row[j] > work_vector[j])
		{
			return false;
		}
	}
	return true;
}


void DetectDeadlock(const vector<vector<int>>& allocation_matrix, const vector<vector<int>>& request_matrix, const vector<int>& available_vector)
{
	int process_count = static_cast<int>(allocation_matrix.size());
	int resource_count = static_cast<int>(available_vector.size());

	vector<int> work_vector = available_vector;
	vector<bool> finished_processes(process_count, false);

	for (int i = 0; i < process_count; i++)
	{
		bool has_allocated = false;
		for (int j = 0; j < resource_count; j++)
		{
			if (allocation_matrix[i][j] != 0)
			{
				has_allocated = true;
				break;
			}
		}
		if (!has_allocated)
		{
			finished_processes[i] = true;
		}
	}

	PrintLine('=', " Ход алгоритма ", 45);
	PrintVector("Начальный вектор Work (A)", work_vector);

	bool found_process = true;
	int step = 1;

	while (found_process)
	{
		found_process = false;

		for (int i = 0; i < process_count; i++)
		{
			if (!finished_processes[i] && CanSatisfyRequest(request_matrix[i], work_vector))
			{
				cout << "Шаг " << step++ << ": процесс P" << i + 1 << " может быть выполнен (R[" << i + 1 << "] <= Work).\n";

				for (int j = 0; j < resource_count; j++)
				{
					work_vector[j] += allocation_matrix[i][j];
				}

				finished_processes[i] = true;
				found_process = true;
				PrintVector("  Новый вектор Work", work_vector);
			}
		}
	}

	PrintLine('=', " Результат ", 49);
	vector<int> deadlocked_processes;

	for (int i = 0; i < process_count; i++)
	{
		if (!finished_processes[i])
		{
			deadlocked_processes.push_back(i + 1);
		}
	}

	if (deadlocked_processes.empty())
	{
		cout << "Взаимоблокировка не обнаружена. Все процессы могут завершиться.\n";
	}
	else
	{
		cout << "Обнаружена взаимоблокировка!\n";
		cout << "Процессы, вовлечённые во взаимоблокировку: ";
		for (size_t i = 0; i < deadlocked_processes.size(); i++)
		{
			cout << "P" << deadlocked_processes[i];
			if (i + 1 < deadlocked_processes.size())
			{
				cout << ", ";
			}
		}
		cout << '\n';
	}
	PrintLine('=', "-", 60);
}


int main()
{
	setlocale(LC_CTYPE, "Russian");

	vector<int> total_resources = { 2, 4, 1, 4, 4 };
	vector<int> available_resources = { 0, 1, 0, 2, 1 };

	vector<vector<int>> allocation_matrix = {
		{ 0, 1, 1, 1, 2 }, 
		{ 0, 1, 0, 1, 0 }, 
		{ 0, 0, 0, 0, 1 }, 
		{ 2, 1, 0, 0, 0 }  
	};

	vector<vector<int>> request_matrix = {
		{ 1, 1, 0, 2, 1 }, 
		{ 0, 1, 0, 2, 1 }, 
		{ 0, 2, 0, 3, 1 }, 
		{ 0, 2, 1, 1, 0 }  
	};

	PrintLine('-', " Исходные данные (Задача 1) ", 32);
	PrintVector("Всего ресурсов E", total_resources);
	PrintVector("Доступно ресурсов A", available_resources);
	PrintMatrix("Матрица распределения C", allocation_matrix);
	PrintMatrix("Матрица запросов R", request_matrix);

	DetectDeadlock(allocation_matrix, request_matrix, available_resources);

	return 0;
}