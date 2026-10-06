#include "win_launch.hpp"

int wmain() {
    try {
        const std::wstring exe = sibling_exe(L"Lab-03x.exe");
        std::vector<HANDLE> children;

        // Deliberately invalid: lpApplicationName contains an argument.
        const std::wstring invalid_application = exe + L" 50";
        launch_child(invalid_application.c_str(), nullptr, L"First", children);

        // Only lpCommandLine is supplied; its buffer must be writable.
        std::wstring second = L"\"" + exe + L"\" 50";
        launch_child(nullptr, &second, L"Second", children);

        // Leading space lets the C runtime parse 50 as argv[1].
        std::wstring third = L" 50";
        launch_child(exe.c_str(), &third, L"Third", children);

        return wait_and_close(children);
    } catch (const std::exception& error) {
        std::cerr << error.what() << '\n';
        return 1;
    }
}
