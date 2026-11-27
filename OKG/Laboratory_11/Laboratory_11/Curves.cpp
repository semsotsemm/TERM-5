#include "Curves.h"

#include <stdexcept>

namespace
{
    int GetPointCount(const CMatrix& X, const CMatrix& Y)
    {
        if (X.cols() != 1 || Y.cols() != 1 || X.rows() != Y.rows() || X.rows() == 0)
        {
            throw std::invalid_argument("Expected two nonempty coordinate columns.");
        }
        return X.rows();
    }
}

double Lagr(CMatrix& X, CMatrix& Y, double x)
{
    const int point_count = GetPointCount(X, Y);
    double value = 0.0;
    for (int i = 0; i < point_count; i++)
    {
        double basis = 1.0;
        for (int j = 0; j < point_count; j++)
        {
            if (j == i)
            {
                continue;
            }

            const double difference = X(i) - X(j);
            if (difference == 0.0)
            {
                throw std::invalid_argument("Lagrange nodes must have distinct X coordinates.");
            }
            basis *= (x - X(j)) / difference;
        }
        value += Y(i) * basis;
    }
    return value;
}

void Bezier(CMatrix& X, CMatrix& Y, CMatrix& XB, CMatrix& YB, int M)
{
    const int point_count = GetPointCount(X, Y);
    if (M < 1)
    {
        throw std::invalid_argument("Bezier requires at least one interval.");
    }

    XB.RedimMatrix(M + 1);
    YB.RedimMatrix(M + 1);
    CMatrix work_x(point_count);
    CMatrix work_y(point_count);
    for (int step = 0; step <= M; step++)
    {
        const double t = static_cast<double>(step) / M;
        for (int i = 0; i < point_count; i++)
        {
            work_x(i) = X(i);
            work_y(i) = Y(i);
        }
        for (int remaining = point_count - 1; remaining > 0; remaining--)
        {
            for (int i = 0; i < remaining; i++)
            {
                work_x(i) += t * (work_x(i + 1) - work_x(i));
                work_y(i) += t * (work_y(i + 1) - work_y(i));
            }
        }
        XB(step) = work_x(0);
        YB(step) = work_y(0);
    }
}
