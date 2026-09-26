#pragma once

#include "CMatrix.h"
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

CMatrix SpaceToWindow(CRectD& rs, CRect& rw);

class CPlot2D
{
private:
    CMatrix X;              // Мировые координаты аргументов Xi.
    CMatrix Y;              // Мировые координаты значений Yi = F(Xi).
    CMatrix K;              // Матрица перехода из мировой системы в оконную.
    CRect RW;               // Прямоугольник рисования в пикселях окна.
    CRectD RS;              // Видимая область мировой системы координат.
    CMyPen PenLine;         // Перо для графика или фигуры.
    CMyPen PenAxis;         // Перо для осей и рамки области.
    bool show_axes{ true }; // Для F3 оси не нужны, поэтому их можно отключить.

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
