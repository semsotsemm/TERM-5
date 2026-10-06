#include <windows.h>

#include <iostream>

int wmain() {
    STARTUPINFOW received{};
    GetStartupInfoW(&received);
    if (!received.lpTitle) {
        std::wcerr << L"No lpTitle received\n";
        return 1;
    }
    std::wcout << L"Child PID=" << GetCurrentProcessId() << L'\n';
    std::wcout << L"Received: " << received.lpTitle << std::endl;
    Sleep(8000);  // Keep the new console visible for the screenshot.
    return 0;
}
