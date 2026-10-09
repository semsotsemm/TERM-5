#include <windows.h>
#include "CBlade.h"

#define IDM_SHOW_BLADE 1001
#define TIMER_ID 1

CBlade blade;
bool isVisible = false;
bool isRotating = false;

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message)
    {
    case WM_CREATE:
    {
        // Создаем простое меню
        HMENU hMenu = CreateMenu();
        HMENU hSubMenu = CreatePopupMenu();
        AppendMenu(hSubMenu, MF_STRING, IDM_SHOW_BLADE, L"Лопасть");
        AppendMenu(hMenu, MF_POPUP, (UINT_PTR)hSubMenu, L"ЛР_Лопасть");
        SetMenu(hWnd, hMenu);
        break;
    }
    case WM_COMMAND:
        if (LOWORD(wParam) == IDM_SHOW_BLADE)
        {
            isVisible = true;
            InvalidateRect(hWnd, NULL, TRUE); // Перерисовываем окно
        }
        break;

    case WM_LBUTTONDBLCLK:
        if (isVisible && !isRotating)
        {
            isRotating = true;
            SetTimer(hWnd, TIMER_ID, 30, NULL); // Запуск таймера каждые 30 мс
        }
        break;

    case WM_RBUTTONDBLCLK:
        if (isRotating)
        {
            isRotating = false;
            KillTimer(hWnd, TIMER_ID); // Остановка таймера
        }
        break;

    case WM_TIMER:
        if (wParam == TIMER_ID)
        {
            // Поворот по часовой стрелке (отрицательный угол)
            blade.Rotate(-3.0);
            InvalidateRect(hWnd, NULL, TRUE);
        }
        break;

    case WM_PAINT:
    {
        PAINTSTRUCT ps;
        HDC hdc = BeginPaint(hWnd, &ps);

        if (isVisible)
        {
            RECT rect;
            GetClientRect(hWnd, &rect);
            // Устанавливаем лопасть ровно по центру окна
            blade.SetCenter(rect.right / 2.0, rect.bottom / 2.0);
            blade.Draw(hdc);
        }

        EndPaint(hWnd, &ps);
        break;
    }
    case WM_DESTROY:
        PostQuitMessage(0);
        break;
    default:
        return DefWindowProc(hWnd, message, wParam, lParam);
    }
    return 0;
}

int APIENTRY wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPWSTR lpCmdLine, int nCmdShow)
{
    WNDCLASSEX wcex = { sizeof(WNDCLASSEX) };
    wcex.style = CS_HREDRAW | CS_VREDRAW | CS_DBLCLKS; // Включаем поддержку двойных кликов
    wcex.lpfnWndProc = WndProc;
    wcex.hInstance = hInstance;
    wcex.hCursor = LoadCursor(nullptr, IDC_ARROW);
    wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    wcex.lpszClassName = L"BladeWindowClass";
    RegisterClassEx(&wcex);

    // Размер области D (2h на 2h)
    int clientSize = static_cast<int>(blade.GetH() * 2.5); // Немного запаса (2.5 вместо 2)
    RECT wr = { 0, 0, clientSize, clientSize };

    // Вычисляем размер окна с учетом рамки, чтобы размер клиентской области был точно clientSize
    AdjustWindowRect(&wr, WS_OVERLAPPEDWINDOW ^ WS_THICKFRAME ^ WS_MAXIMIZEBOX, TRUE);

    // Создаем окно фиксированного размера (убраны стили THICKFRAME и MAXIMIZEBOX)
    HWND hWnd = CreateWindow(L"BladeWindowClass", L"Лабораторная: Лопасть",
        WS_OVERLAPPEDWINDOW ^ WS_THICKFRAME ^ WS_MAXIMIZEBOX,
        CW_USEDEFAULT, CW_USEDEFAULT, wr.right - wr.left, wr.bottom - wr.top,
        nullptr, nullptr, hInstance, nullptr);

    ShowWindow(hWnd, nCmdShow);
    UpdateWindow(hWnd);

    MSG msg;
    while (GetMessage(&msg, nullptr, 0, 0))
    {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
    return (int)msg.wParam;
}