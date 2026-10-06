#include "win_launch.hpp"

int wmain() {
    try {
        const std::wstring exe = sibling_exe(L"Lab-03x.exe");
        std::vector<HANDLE> children;

        // No argument: the child reads the user-level ITER_NUM inherited
        // by this parent from a newly opened command prompt.
        launch_child(exe.c_str(), nullptr, L"First/global", children);

        std::wstring second = L"\"" + exe + L"\" 7";
        launch_child(nullptr, &second, L"Second/argument", children);

        if (!SetEnvironmentVariableW(L"ITER_NUM", L"9")) {
            std::wcerr << L"SetEnvironmentVariableW failed, error="
                       << GetLastError() << L'\n';
            wait_and_close(children);
            return 1;
        }
        launch_child(exe.c_str(), nullptr, L"Third/local", children);

        return wait_and_close(children);
    } catch (const std::exception& error) {
        std::cerr << error.what() << '\n';
        return 1;
    }
}
