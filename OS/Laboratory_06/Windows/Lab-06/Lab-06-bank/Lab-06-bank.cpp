#include <iostream>
#include <windows.h>
#include <vector>
#include <string>
#include <iomanip>


using namespace std;


const int PROCESS_COUNT = 5;
const int RESOURCE_COUNT = 3;

vector<int> total_resources = { 10, 5, 7 };
vector<int> available_resources = { 3, 3, 2 };

vector<vector<int>> allocation_matrix = {
	{ 0, 1, 0 },
	{ 2, 0, 0 },
	{ 3, 0, 2 },
	{ 2, 1, 1 },
	{ 0, 0, 2 } 
};

vector<vector<int>> max_matrix = {
	{ 7, 5, 3 },
	{ 3, 2, 2 },
	{ 9, 0, 2 },
	{ 2, 2, 2 },
	{ 4, 3, 3 } 
};

vector<vector<int>> need_matrix(PROCESS_COUNT, vector<int>(RESOURCE_COUNT, 0));


void PrintLine(char symbol = '-', string line_name = "-", int length = 70)
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


void CalculateNeedMatrix()
{
	for (int i = 0; i < PROCESS_COUNT; i++)
	{
		for (int j = 0; j < RESOURCE_COUNT; j++)
		{
			need_matrix[i][j] = max_matrix[i][j] - allocation_matrix[i][j];
		}
	}
}


void PrintSystemState()
{
	PrintLine('-', " Текущее состояние системы ", 43);
	cout << left << setw(10) << "Процесс" << setw(16) << "Распределено" << setw(16) << "Максимум" << setw(16) << "Необходимо" << '\n';
	cout << left << setw(10) << "" << setw(16) << "A  B  C" << setw(16) << "A  B  C" << setw(16) << "A  B  C" << '\n';

	for (int i = 0; i < PROCESS_COUNT; i++)
	{
		cout << "P" << left << setw(9) << i;

		for (int j = 0; j < RESOURCE_COUNT; j++)
		{
			cout << setw(3) << allocation_matrix[i][j];
		}
		cout << setw(7) << "";

		for (int j = 0; j < RESOURCE_COUNT; j++)
		{
			cout << setw(3) << max_matrix[i][j];
		}
		cout << setw(7) << "";

		for (int j = 0; j < RESOURCE_COUNT; j++)
		{
			cout << setw(3) << need_matrix[i][j];
		}
		cout << '\n';
	}

	cout << "\nДоступно ресурсов (A B C): " << available_resources[0] << " " << available_resources[1] << " " << available_resources[2] << '\n';
	cout << "Всего ресурсов (A B C):    " << total_resources[0] << " " << total_resources[1] << " " << total_resources[2] << '\n';
	PrintLine('-', "-", 70);
}


bool IsSafeState(vector<int>& safe_sequence)
{
	vector<int> work_vector = available_resources;
	vector<bool> finished_processes(PROCESS_COUNT, false);
	safe_sequence.clear();

	while (static_cast<int>(safe_sequence.size()) < PROCESS_COUNT)
	{
		bool found_process = false;

		for (int i = 0; i < PROCESS_COUNT; i++)
		{
			if (!finished_processes[i])
			{
				bool can_allocate = true;
				for (int j = 0; j < RESOURCE_COUNT; j++)
				{
					if (need_matrix[i][j] > work_vector[j])
					{
						can_allocate = false;
						break;
					}
				}

				if (can_allocate)
				{
					for (int j = 0; j < RESOURCE_COUNT; j++)
					{
						work_vector[j] += allocation_matrix[i][j];
					}
					finished_processes[i] = true;
					safe_sequence.push_back(i);
					found_process = true;
				}
			}
		}

		if (!found_process)
		{
			return false;
		}
	}

	return true;
}


void PrintSafeSequence(const vector<int>& safe_sequence)
{
	cout << "Безопасная последовательность: < ";
	for (size_t i = 0; i < safe_sequence.size(); i++)
	{
		cout << "P" << safe_sequence[i];
		if (i + 1 < safe_sequence.size())
		{
			cout << " -> ";
		}
	}
	cout << " >\n";
}


bool RequestResources(int process_id, const vector<int>& request_vector)
{
	cout << "\nЗапрос от P" << process_id << ": (" << request_vector[0] << ", " << request_vector[1] << ", " << request_vector[2] << ")\n";

	if (process_id < 0 || process_id >= PROCESS_COUNT)
	{
		cout << "Ошибка: неверный номер процесса.\n";
		return false;
	}

	for (int j = 0; j < RESOURCE_COUNT; j++)
	{
		if (request_vector[j] < 0)
		{
			cout << "Ошибка: запрос не может содержать отрицательные значения.\n";
			return false;
		}
		if (request_vector[j] > need_matrix[process_id][j])
		{
			cout << "Отказано: запрос превышает максимальные потребности процесса (Request > Need).\n";
			return false;
		}
	}

	for (int j = 0; j < RESOURCE_COUNT; j++)
	{
		if (request_vector[j] > available_resources[j])
		{
			cout << "Отказано: недостаточно свободных ресурсов, процесс должен ждать (Request > Available).\n";
			return false;
		}
	}

	for (int j = 0; j < RESOURCE_COUNT; j++)
	{
		available_resources[j] -= request_vector[j];
		allocation_matrix[process_id][j] += request_vector[j];
		need_matrix[process_id][j] -= request_vector[j];
	}

	vector<int> safe_sequence;
	if (IsSafeState(safe_sequence))
	{
		cout << "Запрос одобрен! Состояние остаётся безопасным.\n";
		PrintSafeSequence(safe_sequence);
		return true;
	}
	else
	{
		for (int j = 0; j < RESOURCE_COUNT; j++)
		{
			available_resources[j] += request_vector[j];
			allocation_matrix[process_id][j] -= request_vector[j];
			need_matrix[process_id][j] += request_vector[j];
		}
		cout << "Отказано: выделение ресурсов приведёт к небезопасному состоянию (выполнен откат).\n";
		return false;
	}
}


int main()
{
	setlocale(LC_CTYPE, "Russian");

	CalculateNeedMatrix();
	PrintSystemState();

	vector<int> safe_sequence;
	if (IsSafeState(safe_sequence))
	{
		cout << "Начальное состояние системы безопасно.\n";
		PrintSafeSequence(safe_sequence);
	}
	else
	{
		cout << "Начальное состояние системы небезопасно!\n";
		return 0;
	}

	while (true)
	{
		cout << "\nВведите номер процесса (0-" << PROCESS_COUNT - 1 << ") или -1 для выхода: ";
		int process_id;
		if (!(cin >> process_id) || process_id == -1)
		{
			break;
		}

		vector<int> request_vector(RESOURCE_COUNT);
		cout << "Введите запрос ресурсов (A B C): ";
		cin >> request_vector[0] >> request_vector[1] >> request_vector[2];

		if (RequestResources(process_id, request_vector))
		{
			PrintSystemState();
		}
	}

	return 0;
}