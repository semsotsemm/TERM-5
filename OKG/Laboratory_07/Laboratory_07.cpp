#include "CPyramid.h"

#include <algorithm>
#include <cmath>
#include <cwchar>
#include <cwctype>

const int IDM_CAMERA = 1001;
const int IDM_HIDDEN = 1002;
const int IDM_WIREFRAME = 1003;
const int IDM_DEFAULT = 1004;
const int IDC_PHI = 2001;
const int IDC_THETA = 2002;

CPyramid pyramid;
bool show_pyramid = false;
bool remove_hidden_faces = false;

HWND CreateDialogControl(HWND dialog, const wchar_t* class_name, const wchar_t* text,
    DWORD style, int x, int y, int width, int height, int control_id = 0)
{
    HWND control = CreateWindowExW(
        class_name[0] == L'E' ? WS_EX_CLIENTEDGE : 0,
        class_name, text, WS_CHILD | WS_VISIBLE | style,
        x, y, width, height, dialog,
        reinterpret_cast<HMENU>(static_cast<INT_PTR>(control_id)),
        GetModuleHandleW(nullptr), nullptr
    );
    SendMessageW(control, WM_SETFONT, reinterpret_cast<WPARAM>(GetStockObject(DEFAULT_GUI_FONT)), TRUE);
    return control;
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

INT_PTR CALLBACK CameraDialogProc(HWND dialog, UINT message, WPARAM w_param, LPARAM l_param)
{
    switch (message)
    {
        case WM_INITDIALOG:
        {
            CPyramid* current_pyramid = reinterpret_cast<CPyramid*>(l_param);
            SetWindowLongPtrW(dialog, DWLP_USER, l_param);
            SetWindowTextW(dialog, L"Текущее положение камеры");

            RECT rect;
            GetClientRect(dialog, &rect);
            const int width = rect.right;
            CreateDialogControl(dialog, L"STATIC", L"r = 10 (не изменяется)", 0, 18, 14, width - 36, 20);
            CreateDialogControl(dialog, L"STATIC", L"φ — азимут, градусы:", 0, 18, 45, 170, 20);
            CreateDialogControl(dialog, L"STATIC", L"θ — угол от оси Z (0–180°):", 0, 18, 79, 185, 20);

            wchar_t value[64];
            swprintf_s(value, L"%.6g", current_pyramid->GetPhi());
            CreateDialogControl(dialog, L"EDIT", value, WS_TABSTOP | ES_AUTOHSCROLL, 210, 41, 95, 25, IDC_PHI);
            swprintf_s(value, L"%.6g", current_pyramid->GetTheta());
            CreateDialogControl(dialog, L"EDIT", value, WS_TABSTOP | ES_AUTOHSCROLL, 210, 75, 95, 25, IDC_THETA);
            CreateDialogControl(dialog, L"BUTTON", L"Применить", WS_TABSTOP | BS_DEFPUSHBUTTON, 95, 122, 110, 30, IDOK);
            CreateDialogControl(dialog, L"BUTTON", L"Отмена", WS_TABSTOP | BS_PUSHBUTTON, 215, 122, 90, 30, IDCANCEL);

            HWND parent = GetParent(dialog);
            RECT parent_rect;
            RECT dialog_rect;
            GetWindowRect(parent, &parent_rect);
            GetWindowRect(dialog, &dialog_rect);
            SetWindowPos(dialog, nullptr,
                parent_rect.left + (parent_rect.right - parent_rect.left - dialog_rect.right + dialog_rect.left) / 2,
                parent_rect.top + (parent_rect.bottom - parent_rect.top - dialog_rect.bottom + dialog_rect.top) / 2,
                0, 0, SWP_NOSIZE | SWP_NOZORDER);

            SetFocus(GetDlgItem(dialog, IDC_PHI));
            SendDlgItemMessageW(dialog, IDC_PHI, EM_SETSEL, 0, -1);
            return FALSE;
        }
        case WM_COMMAND:
        {
            switch (LOWORD(w_param))
            {
                case IDOK:
                {
                    double phi;
                    double theta;
                    CPyramid* current_pyramid = reinterpret_cast<CPyramid*>(GetWindowLongPtrW(dialog, DWLP_USER));
                    if (!ReadAngle(dialog, IDC_PHI, phi) || !ReadAngle(dialog, IDC_THETA, theta) ||
                        !current_pyramid->SetCamera(phi, theta))
                    {
                        MessageBoxW(dialog, L"Введите числовые углы. θ должен быть от 0 до 180 градусов.\n"
                            L"Азимут φ приводится к диапазону 0–360 градусов.",
                            L"Некорректные координаты", MB_OK | MB_ICONWARNING);
                        return TRUE;
                    }
                    EndDialog(dialog, IDOK);
                    return TRUE;
                }
                case IDCANCEL:
                    EndDialog(dialog, IDCANCEL);
                    return TRUE;
            }
            break;
        }
        case WM_CLOSE:
            EndDialog(dialog, IDCANCEL);
            return TRUE;
    }
    return FALSE;
}

void ShowCameraDialog(HWND window)
{
    alignas(DWORD) BYTE template_data[128] = {};
    DLGTEMPLATE* dialog_template = reinterpret_cast<DLGTEMPLATE*>(template_data);
    dialog_template->style = WS_POPUP | WS_CAPTION | WS_SYSMENU | DS_MODALFRAME;
    dialog_template->cx = 220;
    dialog_template->cy = 112;

    if (DialogBoxIndirectParamW(GetModuleHandleW(nullptr), dialog_template, window,
        CameraDialogProc, reinterpret_cast<LPARAM>(&pyramid)) == IDOK)
    {
        InvalidateRect(window, nullptr, FALSE);
    }
}

void PaintWindow(HWND window)
{
    PAINTSTRUCT paint;
    HDC window_dc = BeginPaint(window, &paint);
    RECT rect;
    GetClientRect(window, &rect);

    HDC memory_dc = CreateCompatibleDC(window_dc);
    HBITMAP bitmap = CreateCompatibleBitmap(window_dc, std::max(1L, rect.right), std::max(1L, rect.bottom));
    HGDIOBJ old_bitmap = SelectObject(memory_dc, bitmap);
    FillRect(memory_dc, &rect, static_cast<HBRUSH>(GetStockObject(WHITE_BRUSH)));
    SetMapMode(memory_dc, MM_TEXT);
    SetBkMode(memory_dc, TRANSPARENT);
    SelectObject(memory_dc, GetStockObject(DEFAULT_GUI_FONT));

    wchar_t camera_text[160];
    swprintf_s(camera_text, L"Камера: r = %.0f;  φ = %.1f°;  θ = %.1f°",
        pyramid.GetRadius(), pyramid.GetPhi(), pyramid.GetTheta());
    TextOutW(memory_dc, 16, 14, camera_text, lstrlenW(camera_text));

    if (show_pyramid)
    {
        const wchar_t* mode_text = remove_hidden_faces
            ? L"С удалением невидимых граней" : L"Без удаления невидимых граней";
        TextOutW(memory_dc, 16, 34, mode_text, lstrlenW(mode_text));
        pyramid.Draw(memory_dc, rect, remove_hidden_faces);
    }

    BitBlt(window_dc, 0, 0, rect.right, rect.bottom, memory_dc, 0, 0, SRCCOPY);
    SelectObject(memory_dc, old_bitmap);
    DeleteObject(bitmap);
    DeleteDC(memory_dc);
    EndPaint(window, &paint);
}

LRESULT CALLBACK WindowProc(HWND window, UINT message, WPARAM w_param, LPARAM l_param)
{
    switch (message)
    {
        case WM_CREATE:
        {
            HMENU menu = CreateMenu();
            HMENU pyramid_menu = CreatePopupMenu();
            AppendMenuW(pyramid_menu, MF_STRING, IDM_CAMERA, L"Текущее положение камеры...");
            AppendMenuW(pyramid_menu, MF_SEPARATOR, 0, nullptr);
            AppendMenuW(pyramid_menu, MF_STRING, IDM_HIDDEN, L"С удалением невидимых граней");
            AppendMenuW(pyramid_menu, MF_STRING, IDM_WIREFRAME, L"Без удаления невидимых граней");
            AppendMenuW(pyramid_menu, MF_SEPARATOR, 0, nullptr);
            AppendMenuW(pyramid_menu, MF_STRING, IDM_DEFAULT, L"Положение камеры по умолчанию");
            AppendMenuW(menu, MF_POPUP, reinterpret_cast<UINT_PTR>(pyramid_menu), L"Пирамида");
            SetMenu(window, menu);
            return 0;
        }
        case WM_COMMAND:
        {
            switch (LOWORD(w_param))
            {
                case IDM_CAMERA:
                    ShowCameraDialog(window);
                    return 0;
                case IDM_HIDDEN:
                case IDM_WIREFRAME:
                    show_pyramid = true;
                    remove_hidden_faces = LOWORD(w_param) == IDM_HIDDEN;
                    CheckMenuRadioItem(GetSubMenu(GetMenu(window), 0), IDM_HIDDEN, IDM_WIREFRAME,
                        LOWORD(w_param), MF_BYCOMMAND);
                    InvalidateRect(window, nullptr, FALSE);
                    return 0;
                case IDM_DEFAULT:
                    pyramid.ResetCamera();
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
            info->ptMinTrackSize = {640, 480};
            return 0;
        }
        case WM_DESTROY:
            PostQuitMessage(0);
            return 0;
    }
    return DefWindowProcW(window, message, w_param, l_param);
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
    window_class.hbrBackground = static_cast<HBRUSH>(GetStockObject(WHITE_BRUSH));
    window_class.lpszClassName = L"Laboratory07Pyramid";

    if (!RegisterClassExW(&window_class))
    {
        return 1;
    }

    HWND window = CreateWindowExW(0, window_class.lpszClassName,
        L"Лабораторная работа 7", WS_OVERLAPPEDWINDOW,
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
