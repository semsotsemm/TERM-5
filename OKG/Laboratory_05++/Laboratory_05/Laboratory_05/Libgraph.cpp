#include "Libgraph.h"
#include <cmath>

CMatrix CreateTranslate2D(double dx, double dy)
{
    CMatrix result(3, 3);
    result(0, 0) = 1.0;
    result(1, 1) = 1.0;
    result(2, 2) = 1.0;
    result(0, 2) = dx;
    result(1, 2) = dy;
    return result;
}

CMatrix CreateRotate2D(double fi)
{
    CMatrix result(3, 3);
    const double PI = 3.141592;
    double radians = fi * PI / 180.0;

    double c = std::cos(radians);
    double s = std::sin(radians);

    result(0, 0) = c;
    result(0, 1) = -s;
    result(1, 0) = s;
    result(1, 1) = c;
    result(2, 2) = 1.0;

    return result;
}