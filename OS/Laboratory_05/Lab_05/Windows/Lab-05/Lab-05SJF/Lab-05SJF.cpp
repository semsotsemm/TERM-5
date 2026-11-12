#include <iostream>
#include <vector>

using namespace std;

struct Task
{
	int id;
	int arrival_time;
	int lead_time;
	int remaining_time;
	int waiting_time;
	int end_time;
	int task_priority;
	int turnaround_time;
};

const vector<Task> TASKS = {
	{ 1,   0,  80,  80, 0, 0, 3, 0 },
	{ 2,  20,  45,  45, 0, 0, 5, 0 },
	{ 3,  30,  60,  60, 0, 0, 2, 0 },
	{ 4,  50,  30,  30, 0, 0, 4, 0 },
	{ 5,  70,  90,  90, 0, 0, 1, 0 },
	{ 6, 100,  55,  55, 0, 0, 3, 0 },
	{ 7, 120,  75,  75, 0, 0, 2, 0 },
	{ 8, 150,  40,  40, 0, 0, 5, 0 },
	{ 9, 180,  65,  65, 0, 0, 4, 0 },
	{ 10, 200, 85,  85, 0, 0, 1, 0 }
};

void CheckNewTasks(vector<Task>& current_tasks, const int& current_time)
{
	for (int i = 0; i < TASKS.size(); i++)
	{
		if (TASKS[i].arrival_time == current_time)
		{
			current_tasks.push_back(TASKS[i]);
		}
	}
}

int FindShortestTaskIndex(const vector<Task>& current_tasks)
{
	if (current_tasks.empty())
	{
		return -1;
	}

	int shortest_index = 0;
	for (int i = 1; i < current_tasks.size(); i++)
	{
		if (current_tasks[i].remaining_time < current_tasks[shortest_index].remaining_time)
		{
			shortest_index = i;
		}
	}
	return shortest_index;
}

int main()
{
	vector<Task> current_tasks;
	vector<Task> completed_tasks;
	int current_time = 0;

	cout << "\n--- SHORTEST REMAINING TIME FIRST EXECUTION ---\n";

	while (completed_tasks.size() < TASKS.size())
	{
		CheckNewTasks(current_tasks, current_time);

		int active_task_index = FindShortestTaskIndex(current_tasks);

		if (active_task_index != -1)
		{
			current_tasks[active_task_index].remaining_time--;

			if (current_tasks[active_task_index].remaining_time == 0)
			{
				Task finished_task = current_tasks[active_task_index];

				finished_task.end_time = current_time + 1;
				finished_task.turnaround_time = finished_task.end_time - finished_task.arrival_time;
				finished_task.waiting_time = finished_task.turnaround_time - finished_task.lead_time;

				completed_tasks.push_back(finished_task);

				current_tasks.erase(current_tasks.begin() + active_task_index);
			}
		}

		current_time++;
	}

	double total_waiting_time = 0;
	double total_turnaround_time = 0;
	double total_lead_time = 0;

	cout << "\n--- STATISTICS ---\n";
	for (int i = 0; i < completed_tasks.size(); i++)
	{
		total_waiting_time += completed_tasks[i].waiting_time;
		total_turnaround_time += completed_tasks[i].turnaround_time;
		total_lead_time += completed_tasks[i].lead_time;
		cout << "Task " << completed_tasks[i].id << " | End: " << completed_tasks[i].end_time << " | Wait: " << completed_tasks[i].waiting_time << " | Turnaround: " << completed_tasks[i].turnaround_time << '\n';
	}

	double avg_waiting_time = total_waiting_time / TASKS.size();
	double avg_turnaround_time = total_turnaround_time / TASKS.size();
	double avg_execution_time = total_lead_time / TASKS.size();
	double throughput = (double)TASKS.size() / current_time;

	cout << "\n--- FINAL METRICS ---\n";
	cout << "Average Waiting Time: " << avg_waiting_time << '\n';
	cout << "Average Turnaround Time: " << avg_turnaround_time << '\n';
	cout << "Average Execution Time: " << avg_execution_time << '\n';
	cout << "Throughput (tasks/time): " << throughput << '\n';

	return 0;
}