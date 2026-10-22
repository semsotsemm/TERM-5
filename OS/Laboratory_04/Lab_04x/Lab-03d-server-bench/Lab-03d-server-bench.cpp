#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <limits.h>
#include <string>
#include <vector>
#include <sstream>

bool readInt(const char* text, int& value) {
    char* end = NULL;
    errno = 0;
    long parsed = strtol(text, &end, 10);

    if (errno == ERANGE || end == text || *end != '\0' ||
        parsed < INT_MIN || parsed > INT_MAX) {
        return false;
    }

    value = (int)parsed;
    return true;
}

int compare(const void* a, const void* b) {
    return (*(const int*)a - *(const int*)b);
}

int main(int argc, char* argv[]) {
    if (argc != 5) {
        fprintf(stderr,
            "Usage: Lab-03d-server-bench <K> <M> <L> <R>\n");
        return 1;
    }

    int K, M, L, R;

    if (!readInt(argv[1], K) || !readInt(argv[2], M) ||
        !readInt(argv[3], L) || !readInt(argv[4], R)) {
        fprintf(stderr, "Arguments must be integers.\n");
        return 1;
    }

    if (K <= 0 || M <= 0 || L < 1 || L > R ||
        R > 2097152 || K > R - L + 1) {
        fprintf(stderr, "Invalid arguments or unsupported range.\n");
        return 1;
    }

    std::vector<HANDLE> processes;
    processes.reserve(K);

    SECURITY_ATTRIBUTES sa = {};
    sa.nLength = sizeof(sa);
    sa.bInheritHandle = TRUE;

    HANDLE hRead = NULL;
    HANDLE hWrite = NULL;

    if (!CreatePipe(&hRead, &hWrite, &sa, 0)) {
        fprintf(stderr, "CreatePipe failed: %lu\n", GetLastError());
        return 1;
    }

    if (!SetHandleInformation(hRead, HANDLE_FLAG_INHERIT, 0)) {
        fprintf(stderr, "SetHandleInformation failed: %lu\n",
            GetLastError());
        CloseHandle(hRead);
        CloseHandle(hWrite);
        return 1;
    }

    int range = R - L + 1;
    int base = range / K;
    int remainder = range % K;
    int currentL = L;
    bool success = true;

    for (int i = 0; i < K; ++i) {
        int length = base + (i < remainder ? 1 : 0);
        int currentR = currentL + length - 1;

        std::string command =
            "Lab-03d-client-bench.exe " +
            std::to_string(M) + " " +
            std::to_string(currentL) + " " +
            std::to_string(currentR);

        STARTUPINFOA si = {};
        PROCESS_INFORMATION pi = {};

        si.cb = sizeof(si);
        si.dwFlags = STARTF_USESTDHANDLES;
        si.hStdInput = GetStdHandle(STD_INPUT_HANDLE);
        si.hStdOutput = hWrite;
        si.hStdError = GetStdHandle(STD_ERROR_HANDLE);

        if (!CreateProcessA(NULL, &command[0], NULL, NULL,
            TRUE, 0, NULL, NULL, &si, &pi)) {
            fprintf(stderr, "CreateProcess failed: %lu\n",
                GetLastError());
            success = false;
            break;
        }

        processes.push_back(pi.hProcess);
        CloseHandle(pi.hThread);
        currentL = currentR + 1;
    }

    CloseHandle(hWrite);

    std::string results;
    char buffer[4096];

    for (;;) {
        DWORD bytesRead = 0;

        if (!ReadFile(hRead, buffer, sizeof(buffer),
            &bytesRead, NULL)) {
            DWORD error = GetLastError();

            if (error != ERROR_BROKEN_PIPE) {
                fprintf(stderr, "ReadFile failed: %lu\n", error);
                success = false;
            }
            break;
        }

        if (bytesRead == 0) break;
        results.append(buffer, bytesRead);
    }

    CloseHandle(hRead);

    for (HANDLE process : processes) {
        DWORD exitCode = STILL_ACTIVE;

        if (WaitForSingleObject(process, INFINITE) != WAIT_OBJECT_0) {
            fprintf(stderr, "WaitForSingleObject failed: %lu\n",
                GetLastError());
            success = false;
        }
        else if (!GetExitCodeProcess(process, &exitCode)) {
            fprintf(stderr, "GetExitCodeProcess failed: %lu\n",
                GetLastError());
            success = false;
        }
        else if (exitCode != 0) {
            fprintf(stderr, "Client failed: %lu\n", exitCode);
            success = false;
        }

        CloseHandle(process);
    }

    if (!success) return 1;

    std::vector<int> primes;
    std::istringstream input(results);
    int value;

    while (input >> value) {
        primes.push_back(value);
    }

    if (!input.eof()) {
        fprintf(stderr, "Invalid data received from clients.\n");
        return 1;
    }

    if (!primes.empty()) {
        qsort(primes.data(), primes.size(), sizeof(int), compare);
    }

    printf("Count: %zu\n", primes.size());

    for (int prime : primes) {
        printf("%d ", prime);
    }

    printf("\n");
    return 0;
}