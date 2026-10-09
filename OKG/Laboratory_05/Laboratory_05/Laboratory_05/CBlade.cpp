#include "CBlade.h"
#include <cmath>

CBlade::CBlade() : h(150.0), phi(40.0), R(20.0), angle(0.0), cx(0.0), cy(0.0) {}

void CBlade::SetCenter(double x, double y)
{
    cx = x;
    cy = y;
}

void CBlade::Rotate(double deltaBeta)
{
    angle += deltaBeta;
    if (angle <= -360.0) angle += 360.0; // Держим угол в пределах круга
}

void CBlade::Draw(HDC hdc) const
{
    // Матрицы мирового преобразования: поворот на текущий угол и перенос в центр экрана
    CMatrix rotWorld = CreateRotate2D(angle);
    CMatrix transWorld = CreateTranslate2D(cx, cy);

    const double PI = 3.141592653589793;
    double radPhi = phi * PI / 180.0;
    double w = h * std::tan(radPhi / 2.0);

    // Рисуем 4 лопасти
    for (int i = 0; i < 4; i++)
    {
        // Поворот каждой лопасти на 90 градусов относительно предыдущей
        CMatrix rotLocal = CreateRotate2D(i * 90.0);

        // Результирующая матрица: Локальный поворот -> Мировой поворот -> Мировой перенос
        CMatrix transform = transWorld * rotWorld * rotLocal;

        // Задаем вершины одного треугольника лопасти (направлен вверх) в виде векторов-столбцов 3x1
        CMatrix p1(3, 1), p2(3, 1), p3(3, 1);
        p1(0) = 0;   p1(1) = 0;  p1(2) = 1; // Центр
        p2(0) = -w;  p2(1) = -h; p2(2) = 1; // Левый верхний угол
        p3(0) = w;   p3(1) = -h; p3(2) = 1; // Правый верхний угол

        // Применяем преобразования к вершинам
        CMatrix res1 = transform * p1;
        CMatrix res2 = transform * p2;
        CMatrix res3 = transform * p3;

        POINT pts[3] = {
            { static_cast<LONG>(res1(0)), static_cast<LONG>(res1(1)) },
            { static_cast<LONG>(res2(0)), static_cast<LONG>(res2(1)) },
            { static_cast<LONG>(res3(0)), static_cast<LONG>(res3(1)) }
        };

        // Закрашиваем (чередуем красный и синий)
        HBRUSH brush = CreateSolidBrush(i % 2 == 0 ? RGB(255, 0, 0) : RGB(0, 100, 200));
        HBRUSH oldBrush = (HBRUSH)SelectObject(hdc, brush);
        Polygon(hdc, pts, 3);
        SelectObject(hdc, oldBrush);
        DeleteObject(brush);
    }

    // Центральный круг (зеленый)
    CMatrix centerPt(3, 1);
    centerPt(0) = 0; centerPt(1) = 0; centerPt(2) = 1;
    CMatrix resCenter = transWorld * centerPt; // Центр не вращаем вокруг себя, только переносим

    int ctx = static_cast<int>(resCenter(0));
    int cty = static_cast<int>(resCenter(1));

    HBRUSH cBrush = CreateSolidBrush(RGB(0, 255, 0));
    HBRUSH oBrush = (HBRUSH)SelectObject(hdc, cBrush);
    Ellipse(hdc, ctx - R, cty - R, ctx + R, cty + R);
    SelectObject(hdc, oBrush);
    DeleteObject(cBrush);
}