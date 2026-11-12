#include <iostream>
#include <unistd.h>
#include <sys/time.h>
#include <sys/resource.h>
#include <sched.h>
#include <ctime>

using namespace std;

int main()
{
	clock_t start_time = clock();
	pid_t pid = getpid();

	for (int i = 1; i <= 1000000; i++)
	{
		if (i % 1000 == 0)
		{
			usleep(200000);

			int priority = getpriority(PRIO_PROCESS, 0);
			int cpu = sched_getcpu();

			cout << "Iteration: " << i << '\n';
			cout << "PID: " << pid << '\n';
			cout << "TID: " << pid << '\n';
			cout << "Niceness: " << priority << '\n';
			cout << "CPU: " << cpu << '\n';
			cout << "-----------------------\n";
		}
	}

	clock_t end_time = clock();
	double elapsed_time = double(end_time - start_time) / CLOCKS_PER_SEC;

	cout << "Elapsed time: " << elapsed_time << " seconds\n";

	return 0;
}