#include <windows.h>

#include <cerrno>
#include <climits>
#include <cstdlib>
#include <cwchar>
#include <iostream>
#include <string>

int wmain(int argc, wchar_t* argv[])
{
    std::wstring text;
    if (argc > 1) {
        text = argv[1];
    } else {
        wchar_t value[64]{};
        const DWORD length = GetEnvironmentVariableW(L"ITER_NUM", value, 64);
        if (length == 0 || length >= 64) ExitProcess(0xC0000005);
        text.assign(value, length);
    }

    wchar_t* end = nullptr;
    errno = 0;
    const long count = std::wcstol(text.c_str(), &end, 10);
    if (text.empty() || *end != L'\0' || errno == ERANGE ||
        count <= 0 || count > INT_MAX) {
        ExitProcess(0xC0000005);
    }

    std::wcout << L"Iterations: " << count << std::endl;
    for (long i = 1; i <= count; ++i) {
        std::wcout << L"PID=" << GetCurrentProcessId()
                   << L" iteration=" << i << std::endl;
        Sleep(500);
    }
    return 0;
}
