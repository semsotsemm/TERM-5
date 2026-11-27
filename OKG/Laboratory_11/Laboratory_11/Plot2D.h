#pragma once

#ifndef NOMINMAX
#define NOMINMAX
#endif

#include <afxwin.h>
#include "CMatrix.h"

struct CRectD
{
    double left;
    double top;
    double right;
    double bottom;
};

CMatrix SpaceToWindow(const CRectD& world_rect, const CRect& window_rect);

class CPlot2D
{
public:
    CPlot2D(const CRect& plot_rect, const CMatrix& point_x, const CMatrix& point_y,
        const CMatrix& curve_x, const CMatrix& curve_y, bool equal_scale);

    void DrawAxes(CDC& dc) const;
    void DrawLine(CDC& dc, const CMatrix& X, const CMatrix& Y, COLORREF color,
        int pen_style = PS_SOLID, int width = 2) const;
    void DrawPoints(CDC& dc, const CMatrix& X, const CMatrix& Y, COLORREF color,
        bool show_labels) const;

private:
    CRect window_rect;
    CRectD world_rect;
    CMatrix transform;

    CPoint GetWindowPoint(double x, double y) const;
};
