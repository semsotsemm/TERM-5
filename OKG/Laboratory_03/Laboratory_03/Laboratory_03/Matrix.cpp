#include "Matrix.h"

#include <stdexcept>

CMatrix::CMatrix(int rows, int columns)
{
    RedimMatrix(rows, columns);
}

void CMatrix::RedimMatrix(int rows, int columns)
{
    if (rows < 0 || columns < 0)
    {
        throw std::invalid_argument("Размер матрицы не может быть отрицательным.");
    }

    rows_count = rows;
    columns_count = columns;
    values.assign(static_cast<size_t>(rows) * columns, 0.0);
}

int CMatrix::Rows() const
{
    return rows_count;
}

int CMatrix::Columns() const
{
    return columns_count;
}

double& CMatrix::operator()(int row, int column)
{
    return values.at(static_cast<size_t>(row) * columns_count + column);
}

double CMatrix::operator()(int row, int column) const
{
    return values.at(static_cast<size_t>(row) * columns_count + column);
}
