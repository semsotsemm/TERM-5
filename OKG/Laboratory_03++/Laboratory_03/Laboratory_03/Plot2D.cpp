#include "Plot2D.h"

#include <algorithm>
#include <cmath>
#include <vector>

using namespace std;

HPEN CreateMyPen(const CMyPen& pen)
{
    LOGBRUSH brush{ BS_SOLID, pen.PenColor, 0 };
    DWORD style = PS_GEOMETRIC | static_cast<DWORD>(pen.PenStyle) | PS_ENDCAP_ROUND | PS_JOIN_ROUND;

    return ExtCreatePen(style, max(1, pen.PenWidth), &brush, 0, nullptr);
}

CMatrix SpaceToWindow(CRectD& rs, CRect& rw)
{
    CMatrix result(3, 3);

    double world_width = rs.right - rs.left;
    double world_height = rs.top - rs.bottom;
    if (world_width == 0.0 || world_height == 0.0)
    {
        return result;
    }

    double scale_x = static_cast<double>(rw.Width()) / world_width;
    double scale_y = -static_cast<double>(rw.Height()) / world_height;

    result(0, 0) = scale_x;
    result(0, 2) = rw.left - scale_x * rs.left;
    result(1, 1) = scale_y;
    result(1, 2) = rw.top - scale_y * rs.top;
    result(2, 2) = 1.0;

    return result;
}

CPlot2D::CPlot2D()
{
    K.RedimMatrix(3, 3);
}

void CPlot2D::SetParams(CMatrix& XX, CMatrix& YY, CRect& RWX)
{
    X = XX;
    Y = YY;
    RW = RWX;

    int count = min(X.rows(), Y.rows());

    double max_x = 0.0;
    double max_y = 0.0;
    for (int i = 0; i < count; i++)
    {
        max_x = max(max_x, abs(X(i, 0)));
        max_y = max(max_y, abs(Y(i, 0)));
    }

    if (max_x == 0.0)
    {
        max_x = 1.0;
    }
    if (max_y == 0.0)
    {
        max_y = 1.0;
    }

    RS = CRectD(-max_x * 1.1, max_y * 1.1, max_x * 1.1, -max_y * 1.1);
    K = SpaceToWindow(RS, RW);
}

void CPlot2D::SetWindowRect(CRect& RWX)
{
    RW = RWX;
    K = SpaceToWindow(RS, RW);
}

void CPlot2D::GetWindowCoords(double xs, double ys, int& xw, int& yw)
{
    xw = static_cast<int>(lround(K(0, 0) * xs + K(0, 2)));
    yw = static_cast<int>(lround(K(1, 1) * ys + K(1, 2)));
}

void CPlot2D::SetPenLine(CMyPen& PLine)
{
    PenLine = PLine;
}

void CPlot2D::SetPenAxis(CMyPen& PAxis)
{
    PenAxis = PAxis;
}

void CPlot2D::SetShowAxes(bool value)
{
    show_axes = value;
}

void CPlot2D::Draw(CDC& dc, int Ind1, int Ind2)
{
    HDC hdc = dc.GetSafeHdc();
    int count = min(X.rows(), Y.rows());
    if (hdc == nullptr)
    {
        return;
    }

    int first_index = clamp(Ind1, 0, count - 1);
    int last_index = clamp(Ind2, first_index, count - 1);

    HPEN axis_pen = CreateMyPen(PenAxis);
    HGDIOBJ old_pen = SelectObject(hdc, axis_pen);
    HGDIOBJ old_brush = SelectObject(hdc, GetStockObject(NULL_BRUSH));

    Rectangle(hdc, RW.left, RW.top, RW.right, RW.bottom);

    if (show_axes)
    {
        int x_zero{};
        int y_zero{};
        GetWindowCoords(0.0, 0.0, x_zero, y_zero);

        MoveToEx(hdc, x_zero, RW.top, nullptr);
        LineTo(hdc, x_zero, RW.bottom);
        MoveToEx(hdc, RW.left, y_zero, nullptr);
        LineTo(hdc, RW.right, y_zero);
    }

    HPEN line_pen = CreateMyPen(PenLine);
    SelectObject(hdc, line_pen);

    vector<POINT> points(static_cast<size_t>(last_index - first_index + 1));
    for (int i = first_index; i <= last_index; i++)
    {
        int x_window{};
        int y_window{};
        GetWindowCoords(X(i, 0), Y(i, 0), x_window, y_window);
        points[i - first_index] = POINT{ x_window, y_window };
    }

    if (points.size() > 1)
    {
        Polyline(hdc, points.data(), static_cast<int>(points.size()));
    }

    SelectObject(hdc, old_brush);
    SelectObject(hdc, old_pen);
    DeleteObject(line_pen);
    DeleteObject(axis_pen);
}
