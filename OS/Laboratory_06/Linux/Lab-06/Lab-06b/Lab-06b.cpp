#include <iostream>
#include <unistd.h>
#include <sys/wait.h>
#include <semaphore.h>
#include <fcntl.h>
#include <cstdlib>
#include <string>
#include <clocale>


using namespace std;


const char* SEMAPHORE_NAME = "/Lab-06b-semaphore";
string USERNAME;
sem_t* semaphore = SEM_FAILED;


void ProcessCycle(string process_name)
{
	for (int i = 1; i < 101; i++)
	{
		if (i == 30)
		{
			sem_wait(semaphore);
		}
		cout << "Процесс: " << process_name << ". Поток: main. Итерация: " << i << ". Символ username: " << USERNAME[i % USERNAME.length()] << '\n';
		usleep(100000);
		if (i == 60)
		{
			sem_post(semaphore);
		}
	}
}


int main(int argc, char* argv[])
{
	setlocale(LC_CTYPE, "Russian");
	bool is_parent = false;
	string process_name;

	if (argc == 1)
	{
		is_parent = true;
		process_name = "main";
	}

	const char* env_value = getenv("USER");
	if (env_value != nullptr)
	{
		USERNAME = env_value;
	}
	else
	{
		cerr << "Ошибка получения имени пользователя\n";
		return 1;
	}

	if (is_parent)
	{
		sem_unlink(SEMAPHORE_NAME);
		semaphore = sem_open(SEMAPHORE_NAME, O_CREAT | O_EXCL, 0644, 1);
	}
	else
	{
		semaphore = sem_open(SEMAPHORE_NAME, 0);
	}

	if (semaphore == SEM_FAILED)
	{
		cerr << "Ошибка создания или открытия семафора\n";
		return 1;
	}

	if (is_parent)
	{
		pid_t process_a_pid = fork();
		if (process_a_pid == -1)
		{
			cerr << "Ошибка при создании процесса A\n";
			sem_close(semaphore);
			sem_unlink(SEMAPHORE_NAME);
			return 1;
		}
		if (process_a_pid == 0)
		{
			execl(argv[0], argv[0], "A", nullptr);
			cerr << "Ошибка запуска процесса A\n";
			return 1;
		}

		pid_t process_b_pid = fork();
		if (process_b_pid == -1)
		{
			cerr << "Ошибка при создании процесса B\n";
			sem_close(semaphore);
			sem_unlink(SEMAPHORE_NAME);
			return 1;
		}
		if (process_b_pid == 0)
		{
			execl(argv[0], argv[0], "B", nullptr);
			cerr << "Ошибка запуска процесса B\n";
			return 1;
		}

		ProcessCycle(process_name);

		waitpid(process_a_pid, nullptr, 0);
		waitpid(process_b_pid, nullptr, 0);

		sem_close(semaphore);
		sem_unlink(SEMAPHORE_NAME);
	}
	else
	{
		process_name = argv[1];
		ProcessCycle(process_name);
		sem_close(semaphore);
	}

	return 0;
}