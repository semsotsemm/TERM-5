#include "CPlot3D.h"
#include "resource.h"

#include <algorithm>
#include <cmath>
#include <cwchar>
#include <cwctype>
#include <limits>

namespace
{
    constexpr double GRID_STEP = 0.25;
    constexpr int PLOT_MARGIN = 32;

    CPlot3D surface_plot;

    double EllipticParaboloid(double x, double y)
    {
        return x * x + y * y;
    }

    double HyperbolicParaboloid(double x, double y)
    {
        return x * x - y * y;
    }

    double UpperHemisphere(double x, double y)
    {
        const double squared_height = 9.0 - x * x - y * y;
        return squared_height >= 0.0 ? std::sqrt(squared_height)
            : std::numeric_limits<double>::quiet_NaN();
    }

    bool ReadAngle(HWND dialog, int control_id, double& value)
    {
        wchar_t text[128];
        GetDlgItemTextW(dialog, control_id, text, 128);
        for (wchar_t& symbol : text)
        {
            if (symbol == L',')
            {
                symbol = L'.';
            }
            if (symbol == L'\0')
            {
                break;
            }
        }

        wchar_t* end = nullptr;
        value = std::wcstod(text, &end);
        if (end == text)
        {
            return false;
        }
        while (*end != L'\0' && std::iswspace(*end))
        {
            end++;
        }
        return *end == L'\0' && std::isfinite(value);
    }

    void ShowAngleError(HWND dialog, int control_id, const wchar_t* error_text)
    {
        SetDlgItemTextW(dialog, IDC_CAMERA_ERROR, error_text);
        SetFocus(GetDlgItem(dialog, control_id));
        SendDlgItemMessageW(dialog, control_id, EM_SETSEL, 0, -1);
    }

    INT_PTR CALLBACK CameraDialogProc(HWND dialog, UINT message, WPARAM w_param, LPARAM)
    {
        switch (message)
        {
            case WM_INITDIALOG:
            {
                SendDlgItemMessageW(dialog, IDC_PHI, EM_SETLIMITTEXT, 127, 0);
                SendDlgItemMessageW(dialog, IDC_THETA, EM_SETLIMITTEXT, 127, 0);

                wchar_t angle_text[64];
                swprintf_s(angle_text, L"%.12g", surface_plot.GetPhi());
                SetDlgItemTextW(dialog, IDC_PHI, angle_text);
                swprintf_s(angle_text, L"%.12g", surface_plot.GetTheta());
                SetDlgItemTextW(dialog, IDC_THETA, angle_text);
                return TRUE;
            }
            case WM_COMMAND:
            {
                const UINT control_id = LOWORD(w_param);
                if (control_id == IDCANCEL)
                {
                    EndDialog(dialog, IDCANCEL);
                    return TRUE;
                }

                if (control_id == IDOK)
                {
                    double phi;
                    double theta;
                    if (!ReadAngle(dialog, IDC_PHI, phi))
                    {
                        ShowAngleError(dialog, IDC_PHI, L"Введите числовое значение φ.");
                        return TRUE;
                    }
                    if (!ReadAngle(dialog, IDC_THETA, theta) || !surface_plot.SetCamera(phi, theta))
                    {
                        ShowAngleError(dialog, IDC_THETA, L"Введите θ от 0 до 180 градусов.");
                        return TRUE;
                    }

                    EndDialog(dialog, IDOK);
                    return TRUE;
                }

                if ((control_id == IDC_PHI || control_id == IDC_THETA) && HIWORD(w_param) == EN_CHANGE)
                {
                    SetDlgItemTextW(dialog, IDC_CAMERA_ERROR, L"");
                }
                break;
            }
        }
        return FALSE;
    }

    void ShowCameraDialog(HWND window)
    {
        const INT_PTR result = DialogBoxW(GetModuleHandleW(nullptr),
            MAKEINTRESOURCEW(IDD_CAMERA), window, CameraDialogProc);
        if (result == IDOK)
        {
            InvalidateRect(window, nullptr, FALSE);
        }
        else if (result == -1)
        {
            MessageBoxW(window, L"Не удалось открыть диалог камеры.", L"Ошибка", MB_OK | MB_ICONERROR);
        }
    }

    void SelectSurface(HWND window, UINT command)
    {
        switch (command)
        {
            case IDM_ELLIPTIC_PARABOLOID:
                surface_plot.SetSurface(EllipticParaboloid, -5.0, 5.0, -5.0, 5.0, GRID_STEP, GRID_STEP);
                break;
            case IDM_HYPERBOLIC_PARABOLOID:
                surface_plot.SetSurface(HyperbolicParaboloid, -5.0, 5.0, -5.0, 5.0, GRID_STEP, GRID_STEP);
                break;
            case IDM_HEMISPHERE:
                surface_plot.SetSurface(UpperHemisphere, -3.0, 3.0, -3.0, 3.0, GRID_STEP, GRID_STEP);
                break;
        }

        CheckMenuRadioItem(GetSubMenu(GetMenu(window), 0), IDM_ELLIPTIC_PARABOLOID,
            IDM_HEMISPHERE, command, MF_BYCOMMAND);
        InvalidateRect(window, nullptr, FALSE);
    }

    void DrawScene(HDC hdc, const RECT& client_rect)
    {
        FillRect(hdc, &client_rect, GetSysColorBrush(COLOR_WINDOW));
        SetMapMode(hdc, MM_TEXT);
        SetBkMode(hdc, TRANSPARENT);
        SetTextColor(hdc, GetSysColor(COLOR_WINDOWTEXT));
        SelectObject(hdc, GetStockObject(DEFAULT_GUI_FONT));

        wchar_t camera_text[128];
        swprintf_s(camera_text, L"Камера: r = %.0f,  φ = %.2f°,  θ = %.2f°",
            surface_plot.GetRadius(), surface_plot.GetPhi(), surface_plot.GetTheta());
        TextOutW(hdc, 16, 14, camera_text, lstrlenW(camera_text));

        const RECT plot_rect = {
            PLOT_MARGIN, 54, client_rect.right - PLOT_MARGIN, client_rect.bottom - PLOT_MARGIN
        };
        surface_plot.Draw(hdc, plot_rect);
    }

    void PaintWindow(HWND window)
    {
        PAINTSTRUCT paint;
        HDC window_dc = BeginPaint(window, &paint);
        RECT client_rect;
        GetClientRect(window, &client_rect);

        HDC memory_dc = CreateCompatibleDC(window_dc);
        HBITMAP bitmap = CreateCompatibleBitmap(window_dc,
            std::max(1L, client_rect.right), std::max(1L, client_rect.bottom));
        HGDIOBJ previous_bitmap = memory_dc && bitmap ? SelectObject(memory_dc, bitmap) : nullptr;

        if (previous_bitmap)
        {
            DrawScene(memory_dc, client_rect);
            BitBlt(window_dc, 0, 0, client_rect.right, client_rect.bottom, memory_dc, 0, 0, SRCCOPY);
            SelectObject(memory_dc, previous_bitmap);
        }
        else
        {
            DrawScene(window_dc, client_rect);
        }

        if (bitmap)
        {
            DeleteObject(bitmap);
        }
        if (memory_dc)
        {
            DeleteDC(memory_dc);
        }
        EndPaint(window, &paint);
    }

    LRESULT CALLBACK WindowProc(HWND window, UINT message, WPARAM w_param, LPARAM l_param)
    {
        switch (message)
        {
            case WM_COMMAND:
            {
                switch (LOWORD(w_param))
                {
                    case IDM_CAMERA:
                        ShowCameraDialog(window);
                        return 0;
                    case IDM_ELLIPTIC_PARABOLOID:
                    case IDM_HYPERBOLIC_PARABOLOID:
                    case IDM_HEMISPHERE:
                        SelectSurface(window, LOWORD(w_param));
                        return 0;
                    case IDM_DEFAULT_CAMERA:
                        surface_plot.ResetCamera();
                        InvalidateRect(window, nullptr, FALSE);
                        return 0;
                }
                break;
            }
            case WM_PAINT:
                PaintWindow(window);
                return 0;
            case WM_ERASEBKGND:
                return 1;
            case WM_SIZE:
                InvalidateRect(window, nullptr, FALSE);
                return 0;
            case WM_GETMINMAXINFO:
            {
                MINMAXINFO* info = reinterpret_cast<MINMAXINFO*>(l_param);
                info->ptMinTrackSize = {480, 360};
                return 0;
            }
            case WM_DESTROY:
                PostQuitMessage(0);
                return 0;
        }
        return DefWindowProcW(window, message, w_param, l_param);
    }
}

int APIENTRY wWinMain(HINSTANCE instance, HINSTANCE, LPWSTR, int show_command)
{
    WNDCLASSEXW window_class = {};
    window_class.cbSize = sizeof(window_class);
    window_class.style = CS_HREDRAW | CS_VREDRAW;
    window_class.lpfnWndProc = WindowProc;
    window_class.hInstance = instance;
    window_class.hCursor = LoadCursorW(nullptr, IDC_ARROW);
    window_class.hIcon = LoadIconW(nullptr, IDI_APPLICATION);
    window_class.hbrBackground = GetSysColorBrush(COLOR_WINDOW);
    window_class.lpszMenuName = MAKEINTRESOURCEW(IDR_FIGURE_MENU);
    window_class.lpszClassName = L"Laboratory08Plot3D";

    if (!RegisterClassExW(&window_class))
    {
        return 1;
    }

    HWND window = CreateWindowExW(0, window_class.lpszClassName,
        L"Лабораторная работа 8", WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, CW_USEDEFAULT, 980, 740, nullptr, nullptr, instance, nullptr);
    if (!window)
    {
        MessageBoxW(nullptr, L"Не удалось создать окно приложения.", L"Ошибка", MB_OK | MB_ICONERROR);
        return 1;
    }

    ShowWindow(window, show_command);
    UpdateWindow(window);

    MSG message;
    int result;
    while ((result = GetMessageW(&message, nullptr, 0, 0)) > 0)
    {
        TranslateMessage(&message);
        DispatchMessageW(&message);
    }
    return result == -1 ? 1 : static_cast<int>(message.wParam);
}
