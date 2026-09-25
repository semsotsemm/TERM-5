#include "ChildView.h"
#include "resource.h"

#include <algorithm>
#include <cmath>
#include <numbers>
#include <string>
#include <vector>

using namespace std;

bool CChildView::Create(HINSTANCE instance, int show_command)
{
    const wchar_t window_class_name[] = L"MyPlot2DWindow";

    WNDCLASSEX window_class{ sizeof(WNDCLASSEX) };
    window_class.lpfnWndProc = WindowProc;
    window_class.hInstance = instance;
    window_class.hCursor = LoadCursor(nullptr, IDC_ARROW);
    window_class.hbrBackground = reinterpret_cast<HBRUSH>(COLOR_WINDOW + 1);
    window_class.lpszClassName = window_class_name;
    window_class.style = CS_HREDRAW | CS_VREDRAW;

    if (!RegisterClassEx(&window_class) && GetLastError() != ERROR_CLASS_ALREADY_EXISTS)
    {
        return false;
    }

    HMENU main_menu = CreateMenu();
    HMENU tests_menu = CreatePopupMenu();
    AppendMenu(tests_menu, MF_STRING, ID_TESTS_F1, L"F1");
    AppendMenu(tests_menu, MF_STRING, ID_TESTS_F2, L"F2");
    AppendMenu(tests_menu, MF_STRING, ID_TESTS_F3, L"F3");
    AppendMenu(main_menu, MF_POPUP, reinterpret_cast<UINT_PTR>(tests_menu), L"Tests_F");

    window_handle = CreateWindowEx(
        0,
        window_class_name,
        L"MyPlot2D - лабораторная работа №3",
        WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT,
        CW_USEDEFAULT,
        1100,
        760,
        nullptr,
        main_menu,
        instance,
        this
    );

    if (window_handle == nullptr)
    {
        DestroyMenu(main_menu);
        return false;
    }

    ShowWindow(window_handle, show_command);
    UpdateWindow(window_handle);
    return true;
}

int CChildView::Run()
{
    MSG message{};
    while (GetMessage(&message, nullptr, 0, 0) > 0)
    {
        TranslateMessage(&message);
        DispatchMessage(&message);
    }

    return static_cast<int>(message.wParam);
}

double CChildView::MyF1(double x)
{
    if (abs(x) < 0.0000001)
    {
        return 1.0;
    }

    return sin(x) / x;
}

double CChildView::MyF2(double x)
{
    return sqrt(abs(x)) * sin(x);
}

LRESULT CALLBACK CChildView::WindowProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    CChildView* window = reinterpret_cast<CChildView*>(GetWindowLongPtr(hWnd, GWLP_USERDATA));

    if (message == WM_NCCREATE)
    {
        CREATESTRUCT* create_data = reinterpret_cast<CREATESTRUCT*>(lParam);
        window = static_cast<CChildView*>(create_data->lpCreateParams);
        window->window_handle = hWnd;
        SetWindowLongPtr(hWnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(window));
    }

    if (window != nullptr)
    {
        return window->HandleMessage(message, wParam, lParam);
    }

    return DefWindowProc(hWnd, message, wParam, lParam);
}

LRESULT CChildView::HandleMessage(UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message)
    {
    case WM_COMMAND:
    {
        switch (LOWORD(wParam))
        {
        case ID_TESTS_F1:
            SelectTest(TestMode::F1);
            break;
        case ID_TESTS_F2:
            SelectTest(TestMode::F2);
            break;
        case ID_TESTS_F3:
            SelectTest(TestMode::F3);
            break;
        }
        return 0;
    }
    case WM_PAINT:
    {
        Paint();
        return 0;
    }
    case WM_SIZE:
    {
        InvalidateRect(window_handle, nullptr, false);
        return 0;
    }
    case WM_ERASEBKGND:
    {
        return 1;
    }
    case WM_DESTROY:
    {
        PostQuitMessage(0);
        return 0;
    }
    }

    return DefWindowProc(window_handle, message, wParam, lParam);
}

void CChildView::SelectTest(TestMode test)
{
    current_test = test;
    InvalidateRect(window_handle, nullptr, false);
}

CRect CChildView::GetPlotRect(const RECT& client_rect) const
{
    const int side_margin = 70;
    const int top_margin = 90;
    const int bottom_margin = 55;

    CRect plot_rect(
        side_margin,
        top_margin,
        client_rect.right - side_margin,
        client_rect.bottom - bottom_margin
    );

    if (current_test == TestMode::F3)
    {
        int square_side = max(100, min(plot_rect.Width(), plot_rect.Height()));
        int center_x = client_rect.right / 2;
        int center_y = (top_margin + client_rect.bottom - bottom_margin) / 2;
        plot_rect = CRect(
            center_x - square_side / 2,
            center_y - square_side / 2,
            center_x + square_side / 2,
            center_y + square_side / 2
        );
    }

    return plot_rect;
}

void CChildView::Paint()
{
    PAINTSTRUCT paint_data{};
    HDC window_dc = BeginPaint(window_handle, &paint_data);

    RECT client_rect{};
    GetClientRect(window_handle, &client_rect);
    if (client_rect.right <= 0 || client_rect.bottom <= 0)
    {
        EndPaint(window_handle, &paint_data);
        return;
    }

    // Двойная буферизация убирает мерцание при изменении размера окна.
    HDC memory_dc = CreateCompatibleDC(window_dc);
    HBITMAP bitmap = CreateCompatibleBitmap(window_dc, client_rect.right, client_rect.bottom);
    HGDIOBJ old_bitmap = SelectObject(memory_dc, bitmap);

    FillRect(memory_dc, &client_rect, reinterpret_cast<HBRUSH>(COLOR_WINDOW + 1));
    SetMapMode(memory_dc, MM_TEXT);
    SetBkMode(memory_dc, TRANSPARENT);

    if (current_test == TestMode::F1)
    {
        DrawFunction(memory_dc, client_rect, true);
    }
    else if (current_test == TestMode::F2)
    {
        DrawFunction(memory_dc, client_rect, false);
    }
    else
    {
        DrawOctagon(memory_dc, client_rect);
    }

    BitBlt(window_dc, 0, 0, client_rect.right, client_rect.bottom, memory_dc, 0, 0, SRCCOPY);

    SelectObject(memory_dc, old_bitmap);
    DeleteObject(bitmap);
    DeleteDC(memory_dc);
    EndPaint(window_handle, &paint_data);
}

void CChildView::DrawFunction(HDC hdc, const RECT& client_rect, bool first_function)
{
    const double pi = numbers::pi;
    const double left_border = first_function ? -3.0 * pi : -4.0 * pi;
    const double right_border = first_function ? 3.0 * pi : 4.0 * pi;
    const double step = pi / 36.0;
    const int points_count = static_cast<int>(lround((right_border - left_border) / step)) + 1;

    CMatrix X(points_count, 1);
    CMatrix Y(points_count, 1);

    for (int i = 0; i < points_count; i++)
    {
        X(i, 0) = left_border + i * step;
        Y(i, 0) = first_function ? MyF1(X(i, 0)) : MyF2(X(i, 0));
    }

    CRect plot_rect = GetPlotRect(client_rect);
    CPlot2D plot;
    plot.SetParams(X, Y, plot_rect);

    CMyPen line_pen;
    CMyPen axis_pen;
    if (first_function)
    {
        line_pen.Set(PS_SOLID, 1, RGB(220, 0, 0));
        axis_pen.Set(PS_SOLID, 2, RGB(0, 80, 220));
    }
    else
    {
        line_pen.Set(PS_DASHDOT, 3, RGB(220, 0, 0));
        axis_pen.Set(PS_SOLID, 2, RGB(0, 0, 0));
    }

    plot.SetPenLine(line_pen);
    plot.SetPenAxis(axis_pen);

    CDC dc(hdc);
    plot.Draw(dc, 0, points_count - 1);

    wstring title = first_function
        ? L"F1: y = sin(x) / x,  x ∈ [-3π; 3π],  Δx = π/36"
        : L"F2: y = √|x| · sin(x),  x ∈ [-4π; 4π],  Δx = π/36";

    SetTextColor(hdc, RGB(30, 30, 30));
    TextOut(hdc, 35, 25, title.c_str(), static_cast<int>(title.size()));
}

void CChildView::DrawOctagon(HDC hdc, const RECT& client_rect)
{
    const int vertices_count = 8;
    const double radius = 10.0;
    const double pi = numbers::pi;

    CMatrix X(vertices_count + 1, 1);
    CMatrix Y(vertices_count + 1, 1);

    for (int i = 0; i <= vertices_count; i++)
    {
        double angle = pi / 8.0 + i * 2.0 * pi / vertices_count;
        X(i, 0) = radius * cos(angle);
        Y(i, 0) = radius * sin(angle);
    }

    CRect plot_rect = GetPlotRect(client_rect);
    CPlot2D plot;
    plot.SetParams(X, Y, plot_rect);
    plot.SetShowAxes(false);

    CMyPen figure_pen;
    CMyPen circle_pen;
    figure_pen.Set(PS_SOLID, 3, RGB(220, 0, 0));
    circle_pen.Set(PS_SOLID, 2, RGB(0, 80, 220));
    plot.SetPenLine(figure_pen);
    plot.SetPenAxis(circle_pen);

    CDC dc(hdc);
    plot.Draw(dc, 0, vertices_count);

    HPEN windows_circle_pen = CreatePen(PS_SOLID, 2, RGB(0, 80, 220));
    HGDIOBJ old_pen = SelectObject(hdc, windows_circle_pen);

    vector<POINT> circle_points(361);
    for (int i = 0; i <= 360; i++)
    {
        double angle = i * 2.0 * pi / 360.0;
        int x_window{};
        int y_window{};
        plot.GetWindowCoords(radius * cos(angle), radius * sin(angle), x_window, y_window);
        circle_points[i] = POINT{ x_window, y_window };
    }
    Polyline(hdc, circle_points.data(), static_cast<int>(circle_points.size()));

    SelectObject(hdc, old_pen);
    DeleteObject(windows_circle_pen);

    wstring title = L"F3: правильный восьмиугольник, вписанный в окружность R = 10";
    SetTextColor(hdc, RGB(30, 30, 30));
    TextOut(hdc, 35, 25, title.c_str(), static_cast<int>(title.size()));
}
