#pragma once
#include <windows.h>
#include "CMatrix.h"
#include "Libgraph.h"

class CBlade
{
private:
    double h;     // Высота лопасти
    double phi;   // Угол раствора (в градусах)
    double R;     // Радиус центра
    double angle; // Текущий угол поворота фигуры
    double cx;    // Координата X центра окна
    double cy;    // Координата Y центра окна

public:
    CBlade();
    void SetCenter(double x, double y);
    void Rotate(double deltaBeta);
    void Draw(HDC hdc) const;
    double GetH() const { return h; }
};