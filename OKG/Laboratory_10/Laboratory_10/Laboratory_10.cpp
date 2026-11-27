#include "CLightSphere.h"
#include "resource.h"

#include <algorithm>
#include <commdlg.h>
#include <cmath>
#include <cwchar>
#include <cwctype>

namespace
{
    constexpr int PLOT_MARGIN = 24;

    CLightSphere sphere;
    int model_index = -1;

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
        SetDlgItemTextW(dialog, IDC_ANGLE_ERROR, error_text);
        SetFocus(GetDlgItem(dialog, control_id));
        SendDlgItemMessageW(dialog, control_id, EM_SETSEL, 0, -1);
    }

    INT_PTR CALLBACK AngleDialogProc(HWND dialog, UINT message, WPARAM w_param, LPARAM l_param)
    {
        switch (message)
        {
            case WM_INITDIALOG:
            {
                SetWindowLongPtrW(dialog, DWLP_USER, l_param);
                const bool is_light = l_param != 0;
                SendDlgItemMessageW(dialog, IDC_PHI, EM_SETLIMITTEXT, 127, 0);
                SendDlgItemMessageW(dialog, IDC_THETA, EM_SETLIMITTEXT, 127, 0);

                wchar_t angle_text[64];
                swprintf_s(angle_text, L"%.12g", is_light ? sphere.GetLightPhi() : sphere.GetCameraPhi());
                SetDlgItemTextW(dialog, IDC_PHI, angle_text);
                swprintf_s(angle_text, L"%.12g", is_light ? sphere.GetLightTheta() : sphere.GetCameraTheta());
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
                    if (!ReadAngle(dialog, IDC_THETA, theta))
                    {
                        ShowAngleError(dialog, IDC_THETA, L"Введите θ от 0 до 180 градусов.");
                        return TRUE;
                    }

                    const bool is_light = GetWindowLongPtrW(dialog, DWLP_USER) != 0;
                    const bool valid = is_light ? sphere.SetLight(phi, theta) : sphere.SetCamera(phi, theta);
                    if (!valid)
                    {
                        ShowAngleError(dialog, IDC_THETA, L"Введите θ от 0 до 180 градусов.");
                        return TRUE;
                    }

                    EndDialog(dialog, IDOK);
                    return TRUE;
                }
                if ((control_id == IDC_PHI || control_id == IDC_THETA) && HIWORD(w_param) == EN_CHANGE)
                {
                    SetDlgItemTextW(dialog, IDC_ANGLE_ERROR, L"");
                }
                break;
            }
        }
        return FALSE;
    }

    bool ShowAngleDialog(HWND window, bool is_light)
    {
        const int dialog_id = is_light ? IDD_LIGHT : IDD_CAMERA;
        const INT_PTR result = DialogBoxParamW(AfxGetInstanceHandle(),
            MAKEINTRESOURCEW(dialog_id), window, AngleDialogProc, is_light ? 1 : 0);
        if (result == -1)
        {
            MessageBoxW(window, L"Не удалось открыть диалог координат.", L"Ошибка", MB_OK | MB_ICONERROR);
        }
        return result == IDOK;
    }

    void DrawScene(CDC& dc, const CRect& client_rect)
    {
        dc.FillSolidRect(client_rect, GetSysColor(COLOR_WINDOW));
        if (model_index < 0)
        {
            return;
        }

        dc.SetMapMode(MM_TEXT);
        dc.SetBkMode(TRANSPARENT);
        dc.SetTextColor(GetSysColor(COLOR_WINDOWTEXT));
        dc.SelectStockObject(DEFAULT_GUI_FONT);

        wchar_t coordinates[160];
        swprintf_s(coordinates, L"Источник света: r = %.0f,  φ = %.2f°,  θ = %.2f°",
            sphere.GetLightRadius(), sphere.GetLightPhi(), sphere.GetLightTheta());
        dc.TextOutW(16, 14, coordinates);
        swprintf_s(coordinates, L"Камера: r = %.0f,  φ = %.2f°,  θ = %.2f°",
            sphere.GetCameraRadius(), sphere.GetCameraPhi(), sphere.GetCameraTheta());
        dc.TextOutW(16, 38, coordinates);

        CRect plot_rect(PLOT_MARGIN, 76, client_rect.right - PLOT_MARGIN, client_rect.bottom - PLOT_MARGIN);
        sphere.Draw(dc, plot_rect, model_index);
    }
}

class CMainWindow : public CFrameWnd
{
    protected:
        afx_msg void OnPaint();
        afx_msg BOOL OnEraseBkgnd(CDC* dc);
        afx_msg void OnSize(UINT type, int width, int height);
        afx_msg void OnGetMinMaxInfo(MINMAXINFO* info);
        afx_msg void OnCamera();
        afx_msg void OnLight();
        afx_msg void OnLightColor();
        afx_msg void OnDiffuse();
        afx_msg void OnSpecular();
        void ShowSphere(int index);
        DECLARE_MESSAGE_MAP()
};

BEGIN_MESSAGE_MAP(CMainWindow, CFrameWnd)
    ON_WM_PAINT()
    ON_WM_ERASEBKGND()
    ON_WM_SIZE()
    ON_WM_GETMINMAXINFO()
    ON_COMMAND(IDM_CAMERA, &CMainWindow::OnCamera)
    ON_COMMAND(IDM_LIGHT, &CMainWindow::OnLight)
    ON_COMMAND(IDM_LIGHT_COLOR, &CMainWindow::OnLightColor)
    ON_COMMAND(IDM_DIFFUSE, &CMainWindow::OnDiffuse)
    ON_COMMAND(IDM_SPECULAR, &CMainWindow::OnSpecular)
END_MESSAGE_MAP()

void CMainWindow::OnPaint()
{
    CPaintDC window_dc(this);
    CRect client_rect;
    GetClientRect(&client_rect);

    CDC memory_dc;
    CBitmap bitmap;
    if (memory_dc.CreateCompatibleDC(&window_dc) &&
        bitmap.CreateCompatibleBitmap(&window_dc, std::max(1, client_rect.Width()), std::max(1, client_rect.Height())))
    {
        CBitmap* previous_bitmap = memory_dc.SelectObject(&bitmap);
        if (previous_bitmap != nullptr)
        {
            DrawScene(memory_dc, client_rect);
            window_dc.BitBlt(0, 0, client_rect.Width(), client_rect.Height(), &memory_dc, 0, 0, SRCCOPY);
            memory_dc.SelectObject(previous_bitmap);
            return;
        }
    }
    DrawScene(window_dc, client_rect);
}

BOOL CMainWindow::OnEraseBkgnd(CDC*)
{
    return TRUE;
}

void CMainWindow::OnSize(UINT type, int width, int height)
{
    CFrameWnd::OnSize(type, width, height);
    Invalidate(FALSE);
}

void CMainWindow::OnGetMinMaxInfo(MINMAXINFO* info)
{
    CFrameWnd::OnGetMinMaxInfo(info);
    info->ptMinTrackSize = {480, 360};
}

void CMainWindow::OnCamera()
{
    if (ShowAngleDialog(m_hWnd, false))
    {
        Invalidate(FALSE);
    }
}

void CMainWindow::OnLight()
{
    if (ShowAngleDialog(m_hWnd, true))
    {
        Invalidate(FALSE);
    }
}

void CMainWindow::OnLightColor()
{
    static COLORREF custom_colors[16] = {};
    CHOOSECOLORW color_dialog = {};
    color_dialog.lStructSize = sizeof(color_dialog);
    color_dialog.hwndOwner = m_hWnd;
    color_dialog.rgbResult = sphere.GetLightColor();
    color_dialog.lpCustColors = custom_colors;
    color_dialog.Flags = CC_FULLOPEN | CC_RGBINIT;
    if (ChooseColorW(&color_dialog))
    {
        sphere.SetLightColor(color_dialog.rgbResult);
        Invalidate(FALSE);
    }
}

void CMainWindow::ShowSphere(int index)
{
    model_index = index;
    CMenu* menu = GetMenu();
    menu->CheckMenuItem(IDM_DIFFUSE, MF_BYCOMMAND | (index == 0 ? MF_CHECKED : MF_UNCHECKED));
    menu->CheckMenuItem(IDM_SPECULAR, MF_BYCOMMAND | (index == 1 ? MF_CHECKED : MF_UNCHECKED));
    Invalidate(FALSE);
}

void CMainWindow::OnDiffuse()
{
    ShowSphere(0);
}

void CMainWindow::OnSpecular()
{
    ShowSphere(1);
}

class CLaboratory10App : public CWinApp
{
    public:
        BOOL InitInstance() override
        {
            CWinApp::InitInstance();
            CMainWindow* window = new CMainWindow;
            if (!window->Create(nullptr, L"Лабораторная работа 10", WS_OVERLAPPEDWINDOW,
                CRect(100, 100, 1080, 840), nullptr, MAKEINTRESOURCEW(IDR_FIGURE_MENU)))
            {
                delete window;
                return FALSE;
            }

            m_pMainWnd = window;
            window->ShowWindow(m_nCmdShow);
            window->UpdateWindow();
            return TRUE;
        }
};

CLaboratory10App application;
