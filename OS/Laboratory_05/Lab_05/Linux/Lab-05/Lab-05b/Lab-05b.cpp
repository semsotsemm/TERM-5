#include <iostream>
#include <cstdlib>
#include <unistd.h>
#include <sys/wait.h>
#include <sched.h>
#include <sys/resource.h>

using namespace std;

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

	pid_t pid1 = fork();

	if (pid1 == 0)
	{
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

		setpriority(PRIO_PROCESS, 0, p2);

		execl("./Lab-05x", "Lab-05x", NULL);
		return 1;
	}

	pid_t pid2 = fork();

	if (pid2 == 0)
	{
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

		setpriority(PRIO_PROCESS, 0, p3);

		execl("./Lab-05x", "Lab-05x", NULL);
		return 1;
	}

	waitpid(pid1, NULL, 0);
	waitpid(pid2, NULL, 0);

	return 0;
}