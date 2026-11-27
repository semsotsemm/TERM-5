#include "Plot2D.h"
#include "Curves.h"
#include "resource.h"

#include <algorithm>
#include <cmath>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <limits>
#include <locale>
#include <numbers>
#include <stdexcept>

namespace
{
    constexpr COLORREF CURVE_COLOR = RGB(32, 92, 162);
    constexpr COLORREF NODE_COLOR = RGB(176, 51, 44);
    constexpr COLORREF CONTROL_COLOR = RGB(93, 104, 117);
    constexpr int BEZIER_INTERVALS = 100;

    struct Curve
    {
        CMatrix point_x;
        CMatrix point_y;
        CMatrix curve_x;
        CMatrix curve_y;
    };

    enum ViewMode
    {
        EMPTY_VIEW,
        LAGRANGE_VIEW,
        BEZIER_VIEW
    };

    Curve lagrange;
    Curve bezier[2];
    ViewMode view_mode = EMPTY_VIEW;

    std::filesystem::path GetDataDirectory()
    {
        wchar_t executable_path[32768];
        const DWORD length = GetModuleFileNameW(nullptr, executable_path, _countof(executable_path));
        if (length == 0 || length >= _countof(executable_path))
        {
            throw std::runtime_error("Cannot determine application path.");
        }

        const auto exe_directory = std::filesystem::path(executable_path).parent_path();
        for (auto directory = exe_directory; directory != directory.root_path(); directory = directory.parent_path())
        {
            if (std::filesystem::is_regular_file(directory / L"Laboratory_11.slnx"))
            {
                return directory / L"Mathcad";
            }
        }
        return exe_directory / L"Mathcad";
    }

    void SavePoints(const std::filesystem::path& path, const CMatrix& X, const CMatrix& Y)
    {
        std::ofstream file(path);
        file.exceptions(std::ios::failbit | std::ios::badbit);
        file.imbue(std::locale::classic());
        file << std::setprecision(std::numeric_limits<double>::max_digits10);
        for (int i = 0; i < X.rows(); i++)
        {
            file << X(i) << '\t' << Y(i) << '\n';
        }
        file.close();
    }

    void CalculateLagrange()
    {
        constexpr int node_count = 5;
        constexpr double right_bound = std::numbers::pi;
        constexpr double curve_step = 0.2;
        const int interval_count = static_cast<int>(std::ceil(right_bound / curve_step));

        lagrange.point_x.RedimMatrix(node_count);
        lagrange.point_y.RedimMatrix(node_count);
        for (int i = 0; i < node_count; i++)
        {
            const double x = i * (right_bound / 4.0);
            lagrange.point_x(i) = x;
            lagrange.point_y(i) = std::pow(2.0 + std::cos(x), std::sin(2.0 * x));
        }

        constexpr int plot_intervals = 240;
        lagrange.curve_x.RedimMatrix(plot_intervals + 1);
        lagrange.curve_y.RedimMatrix(plot_intervals + 1);
        for (int i = 0; i <= plot_intervals; i++)
        {
            const double x = right_bound * i / plot_intervals;
            lagrange.curve_x(i) = x;
            lagrange.curve_y(i) = Lagr(lagrange.point_x, lagrange.point_y, x);
        }

        CMatrix sample_x(interval_count + 1);
        CMatrix sample_y(interval_count + 1);
        for (int i = 0; i <= interval_count; i++)
        {
            const double x = std::min(i * curve_step, right_bound);
            sample_x(i) = x;
            sample_y(i) = Lagr(lagrange.point_x, lagrange.point_y, x);
        }

        const auto data_directory = GetDataDirectory();
        std::filesystem::create_directories(data_directory);
        SavePoints(data_directory / L"lagrange_nodes.txt", lagrange.point_x, lagrange.point_y);
        SavePoints(data_directory / L"lagrange_curve.txt", sample_x, sample_y);
    }

    void CalculateBezier(Curve& curve, const double points[][2], int point_count)
    {
        curve.point_x.RedimMatrix(point_count);
        curve.point_y.RedimMatrix(point_count);
        for (int i = 0; i < point_count; i++)
        {
            curve.point_x(i) = points[i][0];
            curve.point_y(i) = points[i][1];
        }
        Bezier(curve.point_x, curve.point_y, curve.curve_x, curve.curve_y, BEZIER_INTERVALS);
    }

    void CalculateBezierVariants()
    {
        const double points_a[][2] = {{0.0, 1.0}, {3.0, 4.0}, {4.0, 0.0}};
        const double points_b[][2] = {{0.0, 1.0}, {2.0, 5.2}, {6.2, 4.5}, {6.8, 0.0}};
        CalculateBezier(bezier[0], points_a, _countof(points_a));
        CalculateBezier(bezier[1], points_b, _countof(points_b));

        const auto data_directory = GetDataDirectory();
        std::filesystem::create_directories(data_directory);
        SavePoints(data_directory / L"bezier_a_points.txt", bezier[0].point_x, bezier[0].point_y);
        SavePoints(data_directory / L"bezier_a_curve.txt", bezier[0].curve_x, bezier[0].curve_y);
        SavePoints(data_directory / L"bezier_b_points.txt", bezier[1].point_x, bezier[1].point_y);
        SavePoints(data_directory / L"bezier_b_curve.txt", bezier[1].curve_x, bezier[1].curve_y);
    }

    void DrawBezierPlot(CDC& dc, const CRect& plot_rect, const Curve& curve, const wchar_t* label)
    {
        dc.TextOutW(plot_rect.left + 8, plot_rect.top - 24, label);
        CPlot2D plot(plot_rect, curve.point_x, curve.point_y, curve.curve_x, curve.curve_y, true);
        plot.DrawAxes(dc);
        plot.DrawLine(dc, curve.point_x, curve.point_y, CONTROL_COLOR, PS_DASH, 1);
        plot.DrawLine(dc, curve.curve_x, curve.curve_y, CURVE_COLOR);
        plot.DrawPoints(dc, curve.point_x, curve.point_y, CONTROL_COLOR, true);
    }

    void DrawScene(CDC& dc, const CRect& client_rect)
    {
        dc.FillSolidRect(client_rect, RGB(255, 255, 255));
        if (view_mode == EMPTY_VIEW || client_rect.Width() < 200 || client_rect.Height() < 180)
        {
            return;
        }

        dc.SetBkMode(TRANSPARENT);
        dc.SetTextColor(RGB(29, 40, 54));
        dc.SelectStockObject(DEFAULT_GUI_FONT);
        const int plot_bottom = client_rect.bottom - 40;
        if (view_mode == LAGRANGE_VIEW)
        {
            dc.TextOutW(24, 15, L"Полином Лагранжа");
            dc.TextOutW(24, 36, L"Синяя линия — полином. Красные точки — узлы функции.");
            const CRect plot_rect(57, 78, client_rect.right - 35, plot_bottom);
            CPlot2D plot(plot_rect, lagrange.point_x, lagrange.point_y, lagrange.curve_x, lagrange.curve_y, false);
            plot.DrawAxes(dc);
            plot.DrawLine(dc, lagrange.curve_x, lagrange.curve_y, CURVE_COLOR);
            plot.DrawPoints(dc, lagrange.point_x, lagrange.point_y, NODE_COLOR, false);
            return;
        }

        dc.TextOutW(24, 15, L"Кривые Безье по рисунку 14.5");
        dc.TextOutW(24, 36, L"Синяя линия — кривая. Серый пунктир — опорная ломаная.");
        const int middle = client_rect.Width() / 2;
        DrawBezierPlot(dc, CRect(57, 94, middle - 31, plot_bottom), bezier[0], L"а) Квадратичная кривая");
        DrawBezierPlot(dc, CRect(middle + 43, 94, client_rect.right - 35, plot_bottom),
            bezier[1], L"б) Кубическая кривая");
    }
}

class CMainWindow : public CFrameWnd
{
protected:
    afx_msg void OnPaint();
    afx_msg BOOL OnEraseBkgnd(CDC* dc);
    afx_msg void OnSize(UINT type, int width, int height);
    afx_msg void OnGetMinMaxInfo(MINMAXINFO* info);
    afx_msg void OnLagrange();
    afx_msg void OnBezier();
    DECLARE_MESSAGE_MAP()

private:
    void ShowCurves(ViewMode mode);
};

BEGIN_MESSAGE_MAP(CMainWindow, CFrameWnd)
    ON_WM_PAINT()
    ON_WM_ERASEBKGND()
    ON_WM_SIZE()
    ON_WM_GETMINMAXINFO()
    ON_COMMAND(IDM_LAGRANGE, &CMainWindow::OnLagrange)
    ON_COMMAND(IDM_BEZIER, &CMainWindow::OnBezier)
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
    info->ptMinTrackSize = {720, 430};
}

void CMainWindow::ShowCurves(ViewMode mode)
{
    try
    {
        if (mode == LAGRANGE_VIEW)
        {
            CalculateLagrange();
        }
        else
        {
            CalculateBezierVariants();
        }
    }
    catch (const std::exception&)
    {
        MessageBoxW(L"Не удалось рассчитать или сохранить данные в папку Mathcad.", L"Ошибка", MB_OK | MB_ICONERROR);
        return;
    }

    view_mode = mode;
    CMenu* menu = GetMenu();
    menu->CheckMenuItem(IDM_LAGRANGE, MF_BYCOMMAND | (mode == LAGRANGE_VIEW ? MF_CHECKED : MF_UNCHECKED));
    menu->CheckMenuItem(IDM_BEZIER, MF_BYCOMMAND | (mode == BEZIER_VIEW ? MF_CHECKED : MF_UNCHECKED));
    Invalidate(FALSE);
}

void CMainWindow::OnLagrange()
{
    ShowCurves(LAGRANGE_VIEW);
}

void CMainWindow::OnBezier()
{
    ShowCurves(BEZIER_VIEW);
}

class CLaboratory11App : public CWinApp
{
public:
    BOOL InitInstance() override
    {
        CWinApp::InitInstance();
        CMainWindow* window = new CMainWindow;
        if (!window->Create(nullptr, L"Лабораторная работа 11", WS_OVERLAPPEDWINDOW,
            CRect(100, 100, 1160, 820), nullptr, MAKEINTRESOURCEW(IDR_LINES_MENU)))
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

CLaboratory11App application;
