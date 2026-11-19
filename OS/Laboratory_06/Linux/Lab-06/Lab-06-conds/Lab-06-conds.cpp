#include <iostream>
#include <pthread.h>
#include <unistd.h>
#include <cstdlib>
#include <cstring>
#include <queue>
#include <string>
#include <ctime>
#include <iomanip>
#include <clocale>


using namespace std;


pthread_mutex_t thread_mutex;
queue<string> buffer;

pthread_cond_t cv_buffer_not_full;
pthread_cond_t cv_buffer_not_empty;

int max_buffer_size = 0;
int min_generator_delay = 0, max_generator_delay = 0;
int min_user_delay = 0, max_user_delay = 0;

unsigned long long app_start_time = 0;

long long total_element_produced_count = 0;
long long period_element_produced_count = 0;

unsigned long long producer_work_time_ms = 0;

long long total_consumed = 0;
long long period_consumed = 0;

unsigned long long consumer_work_time_ms = 0;


unsigned long long GetTimeMs()
{
	timespec ts;
	clock_gettime(CLOCK_MONOTONIC, &ts);
	return static_cast<unsigned long long>(ts.tv_sec) * 1000ULL + ts.tv_nsec / 1000000ULL;
}


void PrintLine(char symbol = '-', string line_name = "-", int length = 100)
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


void* GeneratorThread(void* thread_parameters)
{
	unsigned int seed = static_cast<unsigned int>(time(0)) + static_cast<unsigned int>(pthread_self());
	int item_id = 1;

	while (true)
	{
		int delay = min_generator_delay + rand_r(&seed) % (max_generator_delay - min_generator_delay + 1);
		usleep(delay * 1000);

		pthread_mutex_lock(&thread_mutex);

		producer_work_time_ms += delay;

		while (static_cast<int>(buffer.size()) >= max_buffer_size)
		{
			cout << "[Generator] Блокировка: буфер полон (" << buffer.size() << "/" << max_buffer_size << ")\n";
			pthread_cond_wait(&cv_buffer_not_full, &thread_mutex);
			cout << "[Generator] Пробуждение\n";
		}

		buffer.push("Item_" + to_string(item_id++));
		total_element_produced_count++;
		period_element_produced_count++;

		cout << "[Generator] Элемент добавлен. В буфере: " << buffer.size() << "/" << max_buffer_size << '\n';

		pthread_cond_signal(&cv_buffer_not_empty);
		pthread_mutex_unlock(&thread_mutex);
	}

	return nullptr;
}


void* UserThread(void* thread_parameters)
{
	unsigned int seed = static_cast<unsigned int>(time(0)) + static_cast<unsigned int>(pthread_self()) + 1;

	while (true)
	{
		pthread_mutex_lock(&thread_mutex);

		while (buffer.empty())
		{
			cout << "[User] Блокировка: буфер пуст\n";
			pthread_cond_wait(&cv_buffer_not_empty, &thread_mutex);
			cout << "[User] Пробуждение\n";
		}

		string item = buffer.front();
		buffer.pop();
		total_consumed++;
		period_consumed++;

		cout << "[User] Извлечён " << item << ". В буфере: " << buffer.size() << "/" << max_buffer_size << '\n';

		pthread_cond_signal(&cv_buffer_not_full);
		pthread_mutex_unlock(&thread_mutex);

		int delay = min_user_delay + rand_r(&seed) % (max_user_delay - min_user_delay + 1);
		usleep(delay * 1000);

		pthread_mutex_lock(&thread_mutex);
		consumer_work_time_ms += delay;
		pthread_mutex_unlock(&thread_mutex);
	}

	return nullptr;
}


void* StatisticsThread(void* thread_parameters)
{
	const double period_sec = 5.0;

	while (true)
	{
		sleep(5);

		pthread_mutex_lock(&thread_mutex);

		double uptime_sec = (GetTimeMs() - app_start_time) / 1000.0;
		int cur_size = static_cast<int>(buffer.size());
		double fill_percent = (static_cast<double>(cur_size) / max_buffer_size) * 100.0;

		double prod_work_sec = producer_work_time_ms / 1000.0;
		double avg_prod_time = (total_element_produced_count > 0) ? (static_cast<double>(producer_work_time_ms) / total_element_produced_count) : 0.0;
		double prod_speed = period_element_produced_count / period_sec;

		double cons_work_sec = consumer_work_time_ms / 1000.0;
		double avg_cons_time = (total_consumed > 0) ? (static_cast<double>(consumer_work_time_ms) / total_consumed) : 0.0;
		double cons_speed = period_consumed / period_sec;

		PrintLine('=', " СТАТИСТИКА (5 сек) ", 60);
		cout << fixed << setprecision(2)
			<< "Время работы приложения:               " << uptime_sec << " сек\n"
			<< "Всего произведено элементов:           " << total_element_produced_count << "\n"
			<< "Всего потреблено элементов:            " << total_consumed << "\n"
			<< "Текущее кол-во элементов в буфере:     " << cur_size << "\n"
			<< "Процент заполнения буфера:             " << fill_percent << "%\n"
			<< "--- Производитель ---\n"
			<< "Произведено за последний период:       " << period_element_produced_count << "\n"
			<< "Среднее время между производством:     " << avg_prod_time << " мс\n"
			<< "Текущая скорость производства:         " << prod_speed << " эл/сек\n"
			<< "Общее время работы производителя:      " << prod_work_sec << " сек\n"
			<< "--- Потребитель ---\n"
			<< "Потреблено за последний период:        " << period_consumed << "\n"
			<< "Среднее время обработки элемента:      " << avg_cons_time << " мс\n"
			<< "Текущая скорость потребления:          " << cons_speed << " эл/сек\n"
			<< "Общее время работы потребителя:        " << cons_work_sec << " сек\n";
		PrintLine('=', "-", 80);

		period_element_produced_count = 0;
		period_consumed = 0;

		pthread_mutex_unlock(&thread_mutex);
	}

	return nullptr;
}


bool GetStartupInformation(int argc, char* argv[], string& user_name, int& current_buffer_size)
{
	if ((argc == 2 && strcmp(argv[1], "--help") == 0) || argc != 6)
	{
		cout << "Использование: ./Lab-06-conds [Макс. размер буфера] [Мин. задержка производителя] [Макс. задержка производителя] [Мин. задержка потребителя] [Макс. задержка потребителя]\n"
			<< "Пример: ./Lab-06-conds 10 0 100 50 150\n";
		return false;
	}

	const char* env_value = getenv("USER");
	if (env_value != nullptr)
	{
		user_name = env_value;
	}
	else
	{
		cerr << "Ошибка получения имени пользователя\n";
		return false;
	}

	current_buffer_size = 0;

	try
	{
		max_buffer_size = stoi(argv[1]);
		min_generator_delay = stoi(argv[2]);
		max_generator_delay = stoi(argv[3]);
		min_user_delay = stoi(argv[4]);
		max_user_delay = stoi(argv[5]);
	}
	catch (const exception&)
	{
		cerr << "Ошибка: все аргументы должны быть целыми числами.\n";
		return false;
	}

	if (max_buffer_size <= 0 || min_generator_delay < 0 || max_generator_delay < min_generator_delay || min_user_delay < 0 || max_user_delay < min_user_delay)
	{
		cerr << "Ошибка: некорректные значения параметров (размер буфера > 0, задержки >= 0, min <= max).\n";
		return false;
	}

	return true;
}


int main(int argc, char* argv[])
{
	setlocale(LC_CTYPE, "Russian");

	pthread_mutex_init(&thread_mutex, nullptr);
	pthread_cond_init(&cv_buffer_not_full, nullptr);
	pthread_cond_init(&cv_buffer_not_empty, nullptr);

	string user_name;
	int current_buffer_size;

	if (!GetStartupInformation(argc, argv, user_name, current_buffer_size))
	{
		pthread_cond_destroy(&cv_buffer_not_full);
		pthread_cond_destroy(&cv_buffer_not_empty);
		pthread_mutex_destroy(&thread_mutex);
		return 1;
	}

	PrintLine('-', "Статистика запуска", 22);
	cout << "Имя пользователя: " << user_name << '\n';
	cout << "Текущий размер буфера: " << current_buffer_size << '\n';
	cout << "Максимальный размер буфера: " << max_buffer_size << '\n';
	PrintLine('-', "-", 40);

	app_start_time = GetTimeMs();

	pthread_t thread_generator;
	pthread_t thread_user;

	if (pthread_create(&thread_generator, nullptr, GeneratorThread, nullptr) != 0 ||
		pthread_create(&thread_user, nullptr, UserThread, nullptr) != 0)
	{
		cerr << "Ошибка создания одного из потоков\n";
		pthread_cond_destroy(&cv_buffer_not_full);
		pthread_cond_destroy(&cv_buffer_not_empty);
		pthread_mutex_destroy(&thread_mutex);
		return 1;
	}

	StatisticsThread(nullptr);

	pthread_join(thread_generator, nullptr);
	pthread_join(thread_user, nullptr);

	pthread_cond_destroy(&cv_buffer_not_full);
	pthread_cond_destroy(&cv_buffer_not_empty);
	pthread_mutex_destroy(&thread_mutex);
	return 0;
}