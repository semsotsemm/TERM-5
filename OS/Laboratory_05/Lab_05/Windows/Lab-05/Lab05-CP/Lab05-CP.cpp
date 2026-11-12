#include <iostream>
#include <vector>
#include <queue>

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
    bool in_queue;
};

const vector<Task> TASKS = {
    { 1,   0,  80,  80, 0, 0, 3, 0, false },
    { 2,  20,  45,  45, 0, 0, 5, 0, false },
    { 3,  30,  60,  60, 0, 0, 2, 0, false },
    { 4,  50,  30,  30, 0, 0, 4, 0, false },
    { 5,  70,  90,  90, 0, 0, 1, 0, false },
    { 6, 100,  55,  55, 0, 0, 3, 0, false },
    { 7, 120,  75,  75, 0, 0, 2, 0, false },
    { 8, 150,  40,  40, 0, 0, 5, 0, false },
    { 9, 180,  65,  65, 0, 0, 4, 0, false },
    { 10, 200, 85,  85, 0, 0, 1, 0, false }
};

void CheckNewTasks(vector<Task>& all_tasks, const int& current_time)
{
    for (int i = 0; i < all_tasks.size(); i++)
    {
        if (all_tasks[i].arrival_time == current_time && !all_tasks[i].in_queue && all_tasks[i].remaining_time > 0)
        {
            all_tasks[i].in_queue = true;
        }
    }
}

int FindHighestPriorityTaskIndex(const vector<Task>& all_tasks)
{
    int highest_priority_index = -1;
    int min_priority_value = 9999;

    for (int i = 0; i < all_tasks.size(); i++)
    {
        if (all_tasks[i].in_queue && all_tasks[i].remaining_time > 0)
        {
            if (all_tasks[i].task_priority < min_priority_value)
            {
                min_priority_value = all_tasks[i].task_priority;
                highest_priority_index = i;
            }
        }
    }
    return highest_priority_index;
}

int main()
{
    int time_quantum;
    cout << "Enter time quantum for Preemptive Priority: ";
    cin >> time_quantum;

    vector<Task> all_tasks = TASKS;
    vector<Task> completed_tasks;

    int current_time = 0;
    int active_task_index = -1;
    int current_quantum = 0;

    cout << "\n--- PREEMPTIVE PRIORITY EXECUTION ---\n";

    while (completed_tasks.size() < TASKS.size())
    {
        CheckNewTasks(all_tasks, current_time);

        int best_task_index = FindHighestPriorityTaskIndex(all_tasks);

        if (active_task_index != -1 && best_task_index != -1 && all_tasks[best_task_index].task_priority < all_tasks[active_task_index].task_priority)
        {
            active_task_index = best_task_index;
            current_quantum = 0;
        }
        else if (active_task_index != -1 && current_quantum == time_quantum)
        {
            active_task_index = best_task_index;
            current_quantum = 0;
        }
        else if (active_task_index == -1)
        {
            active_task_index = best_task_index;
            current_quantum = 0;
        }

        if (active_task_index != -1)
        {
            all_tasks[active_task_index].remaining_time--;
            current_quantum++;

            if (all_tasks[active_task_index].remaining_time == 0)
            {
                all_tasks[active_task_index].end_time = current_time + 1;
                all_tasks[active_task_index].turnaround_time = all_tasks[active_task_index].end_time - all_tasks[active_task_index].arrival_time;
                all_tasks[active_task_index].waiting_time = all_tasks[active_task_index].turnaround_time - all_tasks[active_task_index].lead_time;

                completed_tasks.push_back(all_tasks[active_task_index]);
                all_tasks[active_task_index].in_queue = false;
                active_task_index = -1;
                current_quantum = 0;
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