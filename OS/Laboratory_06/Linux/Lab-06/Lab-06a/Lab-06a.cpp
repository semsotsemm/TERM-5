#include <iostream>
#include <pthread.h>
#include <unistd.h>
#include <cstdlib>
#include <string>
#include <clocale>


using namespace std;


string USERNAME;
pthread_mutex_t thread_mutex;


struct ThreadData
{
	string thread_name;
	int iteration_count;
};


void* ThreadFunction(void* thread_parameters)
{
	ThreadData* data = static_cast<ThreadData*>(thread_parameters);

	for (int i = 1; i < data->iteration_count + 1; i++)
	{
		if (i == 30)
		{
			pthread_mutex_lock(&thread_mutex);
		}
		cout << "Поток: " << data->thread_name << ". Итерация: " << i << ". Символ username: " << USERNAME[i % USERNAME.length()] << '\n';
		usleep(100000);
		if (i == 60)
		{
			pthread_mutex_unlock(&thread_mutex);
		}
	}
	return nullptr;
}


int main()
{
	setlocale(LC_CTYPE, "Russian");

	if (pthread_mutex_init(&thread_mutex, nullptr) != 0)
	{
		cerr << "Ошибка инициализации мьютекса\n";
		return 1;
	}

	const char* env_value = getenv("USER");
	if (env_value != nullptr)
	{
		USERNAME = env_value;
	}
	else
	{
		cerr << "Ошибка получения имени пользователя\n";
		pthread_mutex_destroy(&thread_mutex);
		return 1;
	}

	ThreadData thread_a_data = { "A", 100 };
	ThreadData thread_b_data = { "B", 100 };
	ThreadData thread_main_data = { "main", 100 };

	pthread_t thread_a;
	pthread_t thread_b;

	if (pthread_create(&thread_a, nullptr, ThreadFunction, &thread_a_data) != 0 ||
		pthread_create(&thread_b, nullptr, ThreadFunction, &thread_b_data) != 0)
	{
		cerr << "Ошибка создания одного из потоков\n";
		pthread_mutex_destroy(&thread_mutex);
		return 1;
	}

	ThreadFunction(&thread_main_data);

	pthread_join(thread_a, nullptr);
	pthread_join(thread_b, nullptr);

	pthread_mutex_destroy(&thread_mutex);
	return 0;
}