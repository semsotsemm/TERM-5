#include <iostream>
#include <vector>
#include <queue>
#include <cstdlib>
#include <ctime>

using namespace std;

enum TaskType { CPU_BOUND, IO_BOUND };

struct Task {
    int id;
    int arrival_time;
    int lead_time;
    int remaining_time;
    int waiting_time;
    int end_time;
    int turnaround_time;
    TaskType type;
    int current_queue;
    int current_quantum;
    bool in_queue;
    int switches;
    int queue_moves;
};

void GenerateTasks(vector<Task>& all_tasks, int count) {
    srand(time(NULL));
    int current_arrival = 0;

    for (int i = 0; i < count; i++) {
        Task t;
        t.id = i + 1;
        t.arrival_time = current_arrival;
        current_arrival += rand() % 5;

        t.lead_time = (rand() % 100) + 10;
        t.remaining_time = t.lead_time;
        t.waiting_time = 0;
        t.end_time = 0;
        t.turnaround_time = 0;

        t.type = (rand() % 100 < 30) ? IO_BOUND : CPU_BOUND;

        t.current_queue = 0;
        t.current_quantum = 0;
        t.in_queue = false;
        t.switches = 0;
        t.queue_moves = 0;

        all_tasks.push_back(t);
    }
}

void CheckNewTasks(vector<Task>& all_tasks, queue<int>& q0, const int& current_time) {
    for (int i = 0; i < all_tasks.size(); i++) {
        if (all_tasks[i].arrival_time == current_time && !all_tasks[i].in_queue && all_tasks[i].remaining_time > 0) {
            q0.push(i);
            all_tasks[i].in_queue = true;
            all_tasks[i].current_queue = 0;
        }
    }
}

int main() {
    const int Q0_QUANTUM = 15;
    const int Q1_QUANTUM = 60;
    const int TOTAL_TASKS = 100;

    vector<Task> all_tasks;
    GenerateTasks(all_tasks, TOTAL_TASKS);

    queue<int> q0;
    queue<int> q1;
    queue<int> q2;

    vector<Task> completed_q0;
    vector<Task> completed_q1;
    vector<Task> completed_q2;

    int current_time = 0;
    int active_task_index = -1;
    int active_queue = -1;

    int total_switches = 0;
    int total_queue_moves = 0;
    int cpu_active_time = 0;

    cout << "\n--- MULTILEVEL FEEDBACK QUEUE EXECUTION ---\n";

    while (completed_q0.size() + completed_q1.size() + completed_q2.size() < TOTAL_TASKS) {
        CheckNewTasks(all_tasks, q0, current_time);

        if (active_task_index == -1) {
            if (!q0.empty()) {
                active_task_index = q0.front();
                q0.pop();
                active_queue = 0;
                total_switches++;
                all_tasks[active_task_index].switches++;
            }
            else if (!q1.empty()) {
                active_task_index = q1.front();
                q1.pop();
                active_queue = 1;
                total_switches++;
                all_tasks[active_task_index].switches++;
            }
            else if (!q2.empty()) {
                active_task_index = q2.front();
                q2.pop();
                active_queue = 2;
                total_switches++;
                all_tasks[active_task_index].switches++;
            }
        }

        if (active_task_index != -1) {
            cpu_active_time++;
            all_tasks[active_task_index].remaining_time--;
            all_tasks[active_task_index].current_quantum++;

            if (all_tasks[active_task_index].remaining_time == 0) {
                all_tasks[active_task_index].end_time = current_time + 1;
                all_tasks[active_task_index].turnaround_time = all_tasks[active_task_index].end_time - all_tasks[active_task_index].arrival_time;
                all_tasks[active_task_index].waiting_time = all_tasks[active_task_index].turnaround_time - all_tasks[active_task_index].lead_time;

                if (active_queue == 0) completed_q0.push_back(all_tasks[active_task_index]);
                else if (active_queue == 1) completed_q1.push_back(all_tasks[active_task_index]);
                else if (active_queue == 2) completed_q2.push_back(all_tasks[active_task_index]);

                active_task_index = -1;
                active_queue = -1;
            }
            else {
                if (active_queue == 0 && all_tasks[active_task_index].current_quantum == Q0_QUANTUM) {
                    all_tasks[active_task_index].current_quantum = 0;
                    if (all_tasks[active_task_index].type == IO_BOUND) {
                        q0.push(active_task_index);
                    }
                    else {
                        q1.push(active_task_index);
                        all_tasks[active_task_index].current_queue = 1;
                        total_queue_moves++;
                        all_tasks[active_task_index].queue_moves++;
                    }
                    active_task_index = -1;
                    active_queue = -1;
                }
                else if (active_queue == 1 && all_tasks[active_task_index].current_quantum == Q1_QUANTUM) {
                    all_tasks[active_task_index].current_quantum = 0;
                    if (all_tasks[active_task_index].type == IO_BOUND) {
                        q0.push(active_task_index);
                        all_tasks[active_task_index].current_queue = 0;
                        total_queue_moves++;
                        all_tasks[active_task_index].queue_moves++;
                    }
                    else {
                        q2.push(active_task_index);
                        all_tasks[active_task_index].current_queue = 2;
                        total_queue_moves++;
                        all_tasks[active_task_index].queue_moves++;
                    }
                    active_task_index = -1;
                    active_queue = -1;
                }
                else if (active_queue == 2) {
                    if (!q0.empty() || !q1.empty()) {
                        q2.push(active_task_index);
                        active_task_index = -1;
                        active_queue = -1;
                    }
                }
            }
        }
        current_time++;
    }

    auto printQueueStats = [](const vector<Task>& completed, const string& queueName) {
        if (completed.empty()) return;

        double total_wait = 0, total_turnaround = 0;
        cout << "\n--- " << queueName << " RESULTS ---\n";

        for (const auto& t : completed) {
            total_wait += t.waiting_time;
            total_turnaround += t.turnaround_time;
            cout << "PID: " << t.id << " | Arrival: " << t.arrival_time
                << " | Execution Time: " << t.lead_time << " | Wait: " << t.waiting_time
                << " | Turnaround: " << t.turnaround_time << "\n";
        }

        cout << "\nStats for " << queueName << ":\n";
        cout << "Average Waiting Time: " << total_wait / completed.size() << "\n";
        cout << "Average Turnaround Time: " << total_turnaround / completed.size() << "\n";
        };

    printQueueStats(completed_q0, "Q0 (High Priority)");
    printQueueStats(completed_q1, "Q1 (Medium Priority)");
    printQueueStats(completed_q2, "Q2 (Low Priority)");

    double cpu_utilization = (double)cpu_active_time / current_time * 100;

    cout << "\n--- OVERALL STATISTICS ---\n";
    cout << "Total Execution Time: " << current_time << "\n";
    cout << "CPU Utilization: " << cpu_utilization << "%\n";
    cout << "Total Context Switches: " << total_switches << "\n";
    cout << "Total Queue Moves: " << total_queue_moves << "\n";

    return 0;
}