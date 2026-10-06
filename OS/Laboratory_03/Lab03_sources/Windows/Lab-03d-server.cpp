#include <windows.h>

#include <algorithm>
#include <cerrno>
#include <cstdint>
#include <cwchar>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

#include "win_launch.hpp"

struct Child {
    HANDLE process{};
    HANDLE read_pipe{};
    DWORD pid{};
};

static bool parse_positive(const wchar_t* text, std::uint64_t& value) {
    if (!text || !*text || *text == L'-') return false;
    wchar_t* end = nullptr;
    errno = 0;
    const unsigned long long parsed = std::wcstoull(text, &end, 10);
    if (errno == ERANGE || *end != L'\0' || parsed == 0) return false;
    value = parsed;
    return true;
}

int wmain(int argc, wchar_t* argv[]) {
    std::uint64_t k = 0, m = 0, left = 0, right = 0;
    if (argc != 5 || !parse_positive(argv[1], k) ||
        !parse_positive(argv[2], m) || !parse_positive(argv[3], left) ||
        !parse_positive(argv[4], right) || left > right ||
        right - left == UINT64_MAX || k > right - left + 1) {
        std::wcerr << L"Usage: Lab-03d-server K M L R (1<=K<=R-L+1)\n";
        return 1;
    }

    const std::uint64_t length = right - left + 1;
    const std::uint64_t base = length / k;
    const std::uint64_t remainder = length % k;
    const ULONGLONG start_ms = GetTickCount64();
    const std::wstring client = sibling_exe(L"Lab-03d-client.exe");
    std::vector<Child> children;
    bool failed = false;
    std::uint64_t cursor = left;

    // One anonymous pipe per child avoids mixing byte streams from writers.
    // All children are launched before the parent begins reading any pipe.
    for (std::uint64_t i = 0; i < k; ++i) {
        const std::uint64_t part_length = base + ((i == 1) ? remainder : 0);
        const std::uint64_t part_right = cursor + part_length - 1;
        SECURITY_ATTRIBUTES security{sizeof(security), nullptr, TRUE};
        HANDLE read_pipe = nullptr, write_pipe = nullptr;
        if (!CreatePipe(&read_pipe, &write_pipe, &security, 0) ||
            !SetHandleInformation(read_pipe, HANDLE_FLAG_INHERIT, 0)) {
            std::wcerr << L"Pipe setup failed, error=" << GetLastError() << L'\n';
            if (read_pipe) CloseHandle(read_pipe);
            if (write_pipe) CloseHandle(write_pipe);
            failed = true;
            break;
        }

        std::wstring command = L"\"" + client + L"\" " +
            std::to_wstring(m) + L" " + std::to_wstring(cursor) + L" " +
            std::to_wstring(part_right);
        STARTUPINFOW startup{};
        startup.cb = sizeof(startup);
        startup.dwFlags = STARTF_USESTDHANDLES;
        startup.hStdInput = GetStdHandle(STD_INPUT_HANDLE);
        startup.hStdOutput = write_pipe;
        startup.hStdError = GetStdHandle(STD_ERROR_HANDLE);
        PROCESS_INFORMATION info{};
        const BOOL created = CreateProcessW(client.c_str(), command.data(),
            nullptr, nullptr, TRUE, CREATE_NO_WINDOW, nullptr, nullptr,
            &startup, &info);
        CloseHandle(write_pipe);
        if (!created) {
            std::wcerr << L"CreateProcessW failed, error=" << GetLastError() << L'\n';
            CloseHandle(read_pipe);
            failed = true;
            break;
        }
        CloseHandle(info.hThread);
        children.push_back({info.hProcess, read_pipe, info.dwProcessId});
        std::wcout << L"Child PID=" << info.dwProcessId << L" range=["
                   << cursor << L", " << part_right << L"]\n";
        cursor = part_right + 1;
    }

    std::vector<std::uint64_t> all;
    for (Child& child : children) {
        std::string data;
        char buffer[4096];
        for (;;) {
            DWORD bytes = 0;
            if (!ReadFile(child.read_pipe, buffer, sizeof(buffer), &bytes, nullptr)) {
                const DWORD error = GetLastError();
                if (error != ERROR_BROKEN_PIPE) {
                    std::wcerr << L"ReadFile failed, error=" << error << L'\n';
                    failed = true;
                }
                break;
            }
            if (bytes == 0) break;
            data.append(buffer, bytes);
        }
        CloseHandle(child.read_pipe);
        child.read_pipe = nullptr;

        std::istringstream input(data);
        std::uint64_t number = 0;
        while (input >> number) all.push_back(number);
        if (!input.eof()) failed = true;

        if (WaitForSingleObject(child.process, INFINITE) != WAIT_OBJECT_0) {
            failed = true;
        }
        DWORD exit_code = 0;
        if (!GetExitCodeProcess(child.process, &exit_code) || exit_code != 0) {
            failed = true;
        }
        CloseHandle(child.process);
        child.process = nullptr;
    }

    std::sort(all.begin(), all.end());
    std::wcout << L"M=" << m << L" range=[" << left << L", " << right
               << L"] K=" << k << L'\n';
    std::wcout << L"Total found=" << all.size() << L'\n';
    std::wcout << L"Numbers: ";
    for (std::uint64_t n : all) std::wcout << n << L' ';
    std::wcout << L'\n';
    std::wcout << L"Elapsed ms=" << GetTickCount64() - start_ms << L'\n';
    return failed ? 1 : 0;
}
