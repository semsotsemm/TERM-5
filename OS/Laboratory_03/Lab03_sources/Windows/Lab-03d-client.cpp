#include <windows.h>

#include <cerrno>
#include <cstdint>
#include <cwchar>
#include <iostream>
#include <numeric>
#include <sstream>
#include <string>
#include <vector>

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
    std::uint64_t m = 0, left = 0, right = 0;
    if (argc != 4 || !parse_positive(argv[1], m) ||
        !parse_positive(argv[2], left) || !parse_positive(argv[3], right) ||
        left > right) {
        std::wcerr << L"Usage: Lab-03d-client M L R (M>0, 1<=L<=R)\n";
        return 1;
    }

    std::vector<std::uint64_t> found;
    for (std::uint64_t n = left;; ++n) {
        if (std::gcd(n, m) == 1) found.push_back(n);
        if (n == right) break;
    }

    // Optional observation pause; use only while taking Process Explorer shots.
    wchar_t pause_text[32]{};
    if (GetEnvironmentVariableW(L"LAB03_PAUSE_MS", pause_text, 32)) {
        std::uint64_t pause_ms = 0;
        if (parse_positive(pause_text, pause_ms) && pause_ms <= 30000) {
            Sleep(static_cast<DWORD>(pause_ms));
        }
    }

    std::ostringstream message;
    for (std::uint64_t n : found) message << n << ' ';
    const std::string data = message.str();
    HANDLE output = GetStdHandle(STD_OUTPUT_HANDLE);
    if (output == INVALID_HANDLE_VALUE || output == nullptr) return 1;
    std::size_t offset = 0;
    while (offset < data.size()) {
        const DWORD chunk = static_cast<DWORD>(
            (data.size() - offset > 4096) ? 4096 : data.size() - offset);
        DWORD written = 0;
        if (!WriteFile(output, data.data() + offset, chunk, &written, nullptr) ||
            written == 0) {
            std::wcerr << L"WriteFile failed, error=" << GetLastError() << L'\n';
            return 1;
        }
        offset += written;
    }
    return 0;
}
