#include <iostream>
#include <windows.h>
#include <string>
#include <vector>
#include <sstream>
#include <chrono>

using namespace std;

int main(int argc, char* argv[]) {
    if (argc < 5) 
    {
        cout << "Usage: Lab-03d-server <K> <M> <L> <R>\n";
        return 1;
    }

    int K = stoi(argv[1]);
    long long M = stoll(argv[2]);
    long long L = stoll(argv[3]);
    long long R = stoll(argv[4]);

    if (K <= 0 || L > R || M <= 0) 
    {
        cout << "Invalid arguments.\n";
        return 1;
    }

    HANDLE hRead, hWrite;
    SECURITY_ATTRIBUTES sa;         // Настройки безопастности
    ZeroMemory(&sa, sizeof(sa));
    sa.nLength = sizeof(sa);
    sa.bInheritHandle = TRUE;       // Разрешаем дочерним процессам наследовать дескрипторы
    sa.lpSecurityDescriptor = NULL; // Стандартные пд 

    if (!CreatePipe(&hRead, &hWrite, &sa, 0)) 
    {
        cout << "Error: CreatePipe failed. Code: " << GetLastError() << "\n";
        return 1;
    }

    SetHandleInformation(hRead, HANDLE_FLAG_INHERIT, 0); // - Дескр. чтения

    long long range = R - L + 1;
    long long step = range / K;
    long long rem = range % K;

    vector<HANDLE> hProcesses;
    long long current_L = L;

    auto start_time = chrono::high_resolution_clock::now();

    for (int i = 0; i < K; i++) 
    {
        long long current_R = current_L + step - 1;

        if (K == 1) 
        {
            current_R += rem;
        }
        else if (i == 1) 
        {
            current_R += rem;
        }

        if (current_L > current_R)
        {
            break;
        }

        string cmdStr = "Lab-03d-client.exe " + to_string(M) + " " + to_string(current_L) + " " + to_string(current_R);
        char cmd[256];
        strcpy_s(cmd, cmdStr.c_str());

        STARTUPINFOA si;
        PROCESS_INFORMATION pi;
        ZeroMemory(&si, sizeof(si));
        si.cb = sizeof(si);

        si.hStdOutput = hWrite;
        si.dwFlags |= STARTF_USESTDHANDLES;

        if (CreateProcessA(NULL, cmd, NULL, NULL, TRUE, 0, NULL, NULL, &si, &pi)) 
        {
            hProcesses.push_back(pi.hProcess);
            CloseHandle(pi.hThread);
        }
        else 
        {
            cout << "Failed to create process " << i << ". Error: " << GetLastError() << "\n";
        }

        current_L = current_R + 1;
    }

    CloseHandle(hWrite);

    string all_results = "";
    char buffer[4096];
    DWORD bytesRead;

    while (ReadFile(hRead, buffer, sizeof(buffer) - 1, &bytesRead, NULL) && bytesRead > 0) 
    {
        buffer[bytesRead] = '\0';
        all_results += buffer;
    }

    if (!hProcesses.empty()) 
    {
        WaitForMultipleObjects(hProcesses.size(), hProcesses.data(), TRUE, INFINITE);
        for (HANDLE h : hProcesses) 
        {
            CloseHandle(h);
        }
    }
    CloseHandle(hRead);

    auto end_time = chrono::high_resolution_clock::now();
    chrono::duration<double> diff = end_time - start_time;

    int total_count = 0;
    stringstream result_stream(all_results);
    string temp;
    while (result_stream >> temp) 
    {
        total_count++;
    }

    cout << "--- Result ---\n";
    cout << "Number M: " << M << "\n";
    cout << "Range: [" << L << ", " << R << "]\n";
    cout << "Process count K: " << K << "\n";
    cout << "General count of founded numbers: " << total_count << "\n";
    cout << "Found numbers: " << all_results << "\n";
    cout << "Work time: " << diff.count() << " seconds\n";

    system("pause");
    return 0;
}