#include <iostream>
#include <vector>
#include <string>
#include <sstream>
#include <unistd.h>
#include <sys/wait.h>
#include <chrono>

using namespace std;

int main(int argc, char* argv[]) {
    if (argc < 5) {
        cout << "Usage: ./Lab-03d-server <K> <M> <L> <R>\n";
        return 1;
    }

    int K = stoi(argv[1]);
    long long M = stoll(argv[2]);
    long long L = stoll(argv[3]);
    long long R = stoll(argv[4]);

    int pipefd[2];
    if (pipe(pipefd) == -1) {
        perror("pipe failed");
        return 1;
    }

    long long range = R - L + 1;
    long long step = range / K;
    long long rem = range % K;

    vector<pid_t> pids;
    long long current_L = L;
    auto start_time = chrono::high_resolution_clock::now();

    for (int i = 0; i < K; ++i) {
        long long current_R = current_L + step - 1;

        if (K == 1) current_R += rem;
        else if (i == 1) current_R += rem;

        if (current_L > current_R) break;

        pid_t pid = fork();
        if (pid == 0) {
            close(pipefd[0]); 

            dup2(pipefd[1], STDOUT_FILENO);
            close(pipefd[1]);

            string str_M = to_string(M);
            string str_L = to_string(current_L);
            string str_R = to_string(current_R);

            execl("./Lab-03d-client", "Lab-03d-client", str_M.c_str(), str_L.c_str(), str_R.c_str(), NULL);

            perror("execl failed");
            exit(1);
        }
        else if (pid > 0) {
            pids.push_back(pid);
        }
        current_L = current_R + 1;
    }

    close(pipefd[1]);

    string all_results = "";
    char buffer[4096];
    ssize_t bytesRead;

    while ((bytesRead = read(pipefd[0], buffer, sizeof(buffer) - 1)) > 0) {
        buffer[bytesRead] = '\0';
        all_results += buffer;
    }
    close(pipefd[0]);
    
    for (pid_t pid : pids) {
        waitpid(pid, NULL, 0);
    }

    auto end_time = chrono::high_resolution_clock::now();
    chrono::duration<double> diff = end_time - start_time;

    int total_count = 0;
    stringstream ss(all_results);
    string temp;
    while (ss >> temp) total_count++;

    cout << "--- Результы работы сервера ---\n";
    cout << "Число M: " << M << "\n";
    cout << "Диапазон: [" << L << ", " << R << "]\n";
    cout << "Количество процессов K: " << K << "\n";
    cout << "Общее количество найденных чисел: " << total_count << "\n";
    cout << "Найденные числа: " << all_results << "\n";
    cout << "Время работы: " << diff.count() << " секунд\n";

    return 0;
}