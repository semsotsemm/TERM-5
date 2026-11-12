#include <iostream>
#include <cstdlib>
#include <unistd.h>
#include <pthread.h>
#include <sched.h>
#include <sys/resource.h>
#include <sys/time.h>

using namespace std;

void* ThreadFunc(void* arg)
{
	int priority = *(int*)arg;

	pid_t tid = gettid();
	setpriority(PRIO_PROCESS, tid, priority);

	pid_t pid = getpid();

	for (int i = 1; i <= 1000000; i++)
	{
		if (i % 1000 == 0)
		{
			usleep(200000);

			int current_priority = getpriority(PRIO_PROCESS, tid);
			int cpu = sched_getcpu();

			cout << "Iteration: " << i << '\n';
			cout << "PID: " << pid << '\n';
			cout << "TID: " << tid << '\n';
			cout << "Niceness: " << current_priority << '\n';
			cout << "CPU: " << cpu << '\n';
			cout << "-----------------------\n";
		}
	}

	return NULL;
}

int main(int argc, char* argv[])
{
	if (argc != 4)
	{
		cout << "Usage: " << argv[0] << " <P1> <P2> <P3>\n";
		return 1;
	}

	int p1 = atoi(argv[1]);
	int p2 = atoi(argv[2]);
	int p3 = atoi(argv[3]);

	cout << "P1 (Affinity Mask): " << p1 << '\n';
	cout << "P2 (Priority 1): " << p2 << '\n';
	cout << "P3 (Priority 2): " << p3 << '\n';

	if (p1 > 0)
	{
		cpu_set_t mask;
		CPU_ZERO(&mask);
		for (int i = 0; i < sizeof(int) * 8; i++)
		{
			if (p1 & (1 << i))
			{
				CPU_SET(i, &mask);
			}
		}
		sched_setaffinity(0, sizeof(cpu_set_t), &mask);
	}

	pthread_t thread1, thread2;

	pthread_create(&thread1, NULL, ThreadFunc, &p2);
	pthread_create(&thread2, NULL, ThreadFunc, &p3);

	pthread_join(thread1, NULL);
	pthread_join(thread2, NULL);

	return 0;
}