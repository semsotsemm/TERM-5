#include "ChildView.h"

int APIENTRY wWinMain(HINSTANCE instance, HINSTANCE, LPWSTR, int show_command)
{
    CChildView application;
    if (!application.Create(instance, show_command))
    {
        MessageBox(
            nullptr,
            L"Не удалось создать главное окно приложения.",
            L"MyPlot2D",
            MB_OK | MB_ICONERROR
        );
        return 1;
    }

    return application.Run();
}
