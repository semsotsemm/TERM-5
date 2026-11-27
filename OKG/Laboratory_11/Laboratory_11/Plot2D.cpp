#include "Plot2D.h"

#include <algorithm>
#include <cmath>
#include <cwchar>
#include <vector>

namespace
{
    constexpr COLORREF GRID_COLOR = RGB(227, 232, 238);
    constexpr COLORREF AXIS_COLOR = RGB(120, 135, 151);
    constexpr COLORREF LABEL_COLOR = RGB(72, 83, 97);

    double GetTickStep(double span)
    {
        const double step = span / 6.0;
        const double unit = std::pow(10.0, std::floor(std::log10(step)));
        const double fraction = step / unit;
        if (fraction <= 1.0)
        {
            return unit;
        }
        if (fraction <= 2.0)
        {
            return 2.0 * unit;
        }
        if (fraction <= 5.0)
        {
            return 5.0 * unit;
        }
        return 10.0 * unit;
    }
}

CMatrix SpaceToWindow(const CRectD& world_rect, const CRect& window_rect)
{
    CMatrix result(3, 3);
    const double scale_x = window_rect.Width() / (world_rect.right - world_rect.left);
    const double scale_y = -window_rect.Height() / (world_rect.top - world_rect.bottom);

    result(0, 0) = scale_x;
    result(0, 2) = window_rect.left - scale_x * world_rect.left;
    result(1, 1) = scale_y;
    result(1, 2) = window_rect.top - scale_y * world_rect.top;
    result(2, 2) = 1.0;
    return result;
}

CPlot2D::CPlot2D(const CRect& plot_rect, const CMatrix& point_x, const CMatrix& point_y,
    const CMatrix& curve_x, const CMatrix& curve_y, bool equal_scale)
    : window_rect(plot_rect), world_rect{}, transform(3, 3)
{
    const double min_x = std::min({0.0, point_x.MinElement(), curve_x.MinElement()});
    const double max_x = std::max({0.0, point_x.MaxElement(), curve_x.MaxElement()});
    const double min_y = std::min({0.0, point_y.MinElement(), curve_y.MinElement()});
    const double max_y = std::max({0.0, point_y.MaxElement(), curve_y.MaxElement()});
    const double center_x = (min_x + max_x) / 2.0;
    const double center_y = (min_y + max_y) / 2.0;
    double half_width = std::max(0.1, (max_x - min_x) * 0.60);
    double half_height = std::max(0.1, (max_y - min_y) * 0.60);

    if (equal_scale)
    {
        const double ratio = static_cast<double>(window_rect.Width()) / window_rect.Height();
        half_width = std::max(half_width, half_height * ratio);
        half_height = half_width / ratio;
    }

    world_rect = {center_x - half_width, center_y + half_height,
        center_x + half_width, center_y - half_height};
    transform = SpaceToWindow(world_rect, window_rect);
}

CPoint CPlot2D::GetWindowPoint(double x, double y) const
{
    return CPoint(static_cast<int>(std::lround(transform(0, 0) * x + transform(0, 2))),
        static_cast<int>(std::lround(transform(1, 1) * y + transform(1, 2))));
}

void CPlot2D::DrawAxes(CDC& dc) const
{
    dc.FillSolidRect(window_rect, RGB(255, 255, 255));
    CPen grid_pen(PS_SOLID, 1, GRID_COLOR);
    CPen axis_pen(PS_SOLID, 1, AXIS_COLOR);
    CPen* previous_pen = dc.SelectObject(&grid_pen);
    CGdiObject* previous_brush = dc.SelectStockObject(NULL_BRUSH);
    const COLORREF previous_color = dc.SetTextColor(LABEL_COLOR);

    const double step_x = GetTickStep(world_rect.right - world_rect.left);
    const double step_y = GetTickStep(world_rect.top - world_rect.bottom);
    wchar_t label[32];
    for (double x = std::ceil(world_rect.left / step_x) * step_x; x <= world_rect.right; x += step_x)
    {
        const int pixel_x = GetWindowPoint(x, 0.0).x;
        dc.MoveTo(pixel_x, window_rect.top);
        dc.LineTo(pixel_x, window_rect.bottom);
        swprintf_s(label, L"%.4g", std::abs(x) < step_x * 0.001 ? 0.0 : x);
        const int label_width = dc.GetTextExtent(label).cx;
        dc.TextOutW(pixel_x - label_width / 2, window_rect.bottom + 5, label);
    }

    for (double y = std::ceil(world_rect.bottom / step_y) * step_y; y <= world_rect.top; y += step_y)
    {
        const int pixel_y = GetWindowPoint(0.0, y).y;
        dc.MoveTo(window_rect.left, pixel_y);
        dc.LineTo(window_rect.right, pixel_y);
        swprintf_s(label, L"%.4g", std::abs(y) < step_y * 0.001 ? 0.0 : y);
        const int label_width = dc.GetTextExtent(label).cx;
        dc.TextOutW(window_rect.left - label_width - 6, pixel_y - 8, label);
    }

    dc.SelectObject(&axis_pen);
    const CPoint origin = GetWindowPoint(0.0, 0.0);
    dc.MoveTo(origin.x, window_rect.top);
    dc.LineTo(origin.x, window_rect.bottom);
    dc.MoveTo(window_rect.left, origin.y);
    dc.LineTo(window_rect.right, origin.y);
    dc.Rectangle(window_rect);
    dc.TextOutW(window_rect.right + 8, window_rect.bottom + 5, L"X");
    dc.TextOutW(window_rect.left - 19, window_rect.top - 19, L"Y");

    dc.SetTextColor(previous_color);
    dc.SelectObject(previous_brush);
    dc.SelectObject(previous_pen);
}

void CPlot2D::DrawLine(CDC& dc, const CMatrix& X, const CMatrix& Y, COLORREF color,
    int pen_style, int width) const
{
    std::vector<POINT> points;
    points.reserve(X.rows());
    for (int i = 0; i < X.rows(); i++)
    {
        points.push_back(GetWindowPoint(X(i), Y(i)));
    }

    CPen line_pen(pen_style, width, color);
    CPen* previous_pen = dc.SelectObject(&line_pen);
    dc.Polyline(points.data(), static_cast<int>(points.size()));
    dc.SelectObject(previous_pen);
}

void CPlot2D::DrawPoints(CDC& dc, const CMatrix& X, const CMatrix& Y, COLORREF color,
    bool show_labels) const
{
    CPen point_pen(PS_SOLID, 1, color);
    CBrush point_brush(color);
    CPen* previous_pen = dc.SelectObject(&point_pen);
    CBrush* previous_brush = dc.SelectObject(&point_brush);
    const COLORREF previous_color = dc.SetTextColor(color);

    for (int i = 0; i < X.rows(); i++)
    {
        const CPoint point = GetWindowPoint(X(i), Y(i));
        dc.Ellipse(point.x - 4, point.y - 4, point.x + 5, point.y + 5);
        if (show_labels)
        {
            wchar_t label[16];
            swprintf_s(label, L"P%d", i);
            const int label_width = dc.GetTextExtent(label).cx;
            const int label_x = std::min(static_cast<int>(point.x + 7), static_cast<int>(window_rect.right) - label_width - 2);
            const int label_y = point.y - 19 >= window_rect.top ? point.y - 19 : point.y + 7;
            dc.TextOutW(label_x, label_y, label);
        }
    }

    dc.SetTextColor(previous_color);
    dc.SelectObject(previous_brush);
    dc.SelectObject(previous_pen);
}
