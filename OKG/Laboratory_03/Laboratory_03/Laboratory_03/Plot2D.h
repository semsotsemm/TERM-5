#pragma once

#include "Matrix.h"
#include "WinTypes.h"

struct CMyPen
{
    int PenStyle;
    int PenWidth;
    COLORREF PenColor;

    CMyPen()
    {
        PenStyle = PS_SOLID;
        PenWidth = 1;
        PenColor = RGB(0, 0, 0);
    }

    void Set(int PS, int PW, COLORREF PC)
    {
        PenStyle = PS;
        PenWidth = PW;
        PenColor = PC;
    }
};

// Возвращает матрицу пересчета координат из мировых в оконные.
CMatrix SpaceToWindow(CRectD& rs, CRect& rw);

class CPlot2D
{
private:
    CMatrix X;
    CMatrix Y;
    CMatrix K;
    CRect RW;
    CRectD RS;
    CMyPen PenLine;
    CMyPen PenAxis;
    bool show_axes{ true };

public:
    CPlot2D();

    void SetParams(CMatrix& XX, CMatrix& YY, CRect& RWX);
    void SetWindowRect(CRect& RWX);
    void GetWindowCoords(double xs, double ys, int& xw, int& yw);
    void SetPenLine(CMyPen& PLine);
    void SetPenAxis(CMyPen& PAxis);
    void SetShowAxes(bool value);
    void Draw(CDC& dc, int Ind1, int Ind2);
};
