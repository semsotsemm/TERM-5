#include "CMatrix.h"

#include <algorithm>
#include <stdexcept>
#include <utility>

using namespace std;

void CMatrix::Allocate(int rows_count, int columns_count)
{
    if (rows_count <= 0 || columns_count <= 0)
    {
        throw invalid_argument("Размеры матрицы должны быть больше нуля.");
    }

    n_rows = rows_count;
    n_cols = columns_count;
    array = new double* [n_rows] {};

    try
    {
        for (int i = 0; i < n_rows; i++)
        {
            array[i] = new double[n_cols] {};
        }
    }
    catch (...)
    {
        for (int i = 0; i < n_rows; i++)
        {
            delete[] array[i];
        }
        delete[] array;
        array = nullptr;
        throw;
    }
}

void CMatrix::Free()
{
    if (array == nullptr)
    {
        return;
    }

    for (int i = 0; i < n_rows; i++)
    {
        delete[] array[i];
    }
    delete[] array;
    array = nullptr;
}

void CMatrix::Swap(CMatrix& matrix)
{
    swap(array, matrix.array);
    swap(n_rows, matrix.n_rows);
    swap(n_cols, matrix.n_cols);
}

void CMatrix::CheckIndex(int row, int column) const
{
    if (row < 0 || row >= n_rows || column < 0 || column >= n_cols)
    {
        throw out_of_range("Индекс элемента выходит за размеры матрицы.");
    }
}

CMatrix::CMatrix() : array(nullptr), n_rows(0), n_cols(0)
{
    Allocate(1, 1);
}

CMatrix::CMatrix(int rows_count, int columns_count)
    : array(nullptr), n_rows(0), n_cols(0)
{
    Allocate(rows_count, columns_count);
}

CMatrix::CMatrix(int rows_count) : array(nullptr), n_rows(0), n_cols(0)
{
    Allocate(rows_count, 1);
}

CMatrix::CMatrix(const CMatrix& matrix) : array(nullptr), n_rows(0), n_cols(0)
{
    Allocate(matrix.n_rows, matrix.n_cols);

    for (int i = 0; i < n_rows; i++)
    {
        for (int j = 0; j < n_cols; j++)
        {
            array[i][j] = matrix.array[i][j];
        }
    }
}

CMatrix::~CMatrix()
{
    Free();
}

double& CMatrix::operator()(int row, int column)
{
    CheckIndex(row, column);
    return array[row][column];
}

const double& CMatrix::operator()(int row, int column) const
{
    CheckIndex(row, column);
    return array[row][column];
}

double& CMatrix::operator()(int row)
{
    if (n_cols != 1)
    {
        throw logic_error("Один индекс можно использовать только для вектора-столбца.");
    }
    return (*this)(row, 0);
}

const double& CMatrix::operator()(int row) const
{
    if (n_cols != 1)
    {
        throw logic_error("Один индекс можно использовать только для вектора-столбца.");
    }
    return (*this)(row, 0);
}

CMatrix CMatrix::operator-() const
{
    CMatrix result(n_rows, n_cols);
    for (int i = 0; i < n_rows; i++)
    {
        for (int j = 0; j < n_cols; j++)
        {
            result(i, j) = -array[i][j];
        }
    }
    return result;
}

CMatrix& CMatrix::operator=(const CMatrix& matrix)
{
    if (this == &matrix)
    {
        return *this;
    }

    // Копия сначала создается отдельно: при ошибке текущая матрица не испортится.
    CMatrix copy(matrix);
    Swap(copy);
    return *this;
}

CMatrix CMatrix::operator*(const CMatrix& matrix) const
{
    if (n_cols != matrix.n_rows)
    {
        throw invalid_argument("Для умножения число столбцов первой матрицы должно равняться числу строк второй.");
    }

    CMatrix result(n_rows, matrix.n_cols);
    for (int i = 0; i < n_rows; i++)
    {
        for (int j = 0; j < matrix.n_cols; j++)
        {
            for (int k = 0; k < n_cols; k++)
            {
                result(i, j) += array[i][k] * matrix.array[k][j];
            }
        }
    }
    return result;
}

CMatrix CMatrix::operator+(const CMatrix& matrix) const
{
    if (n_rows != matrix.n_rows || n_cols != matrix.n_cols)
    {
        throw invalid_argument("Для сложения размеры матриц должны совпадать.");
    }

    CMatrix result(*this);
    for (int i = 0; i < n_rows; i++)
    {
        for (int j = 0; j < n_cols; j++)
        {
            result(i, j) += matrix(i, j);
        }
    }
    return result;
}

CMatrix CMatrix::operator-(const CMatrix& matrix) const
{
    if (n_rows != matrix.n_rows || n_cols != matrix.n_cols)
    {
        throw invalid_argument("Для вычитания размеры матриц должны совпадать.");
    }

    CMatrix result(*this);
    for (int i = 0; i < n_rows; i++)
    {
        for (int j = 0; j < n_cols; j++)
        {
            result(i, j) -= matrix(i, j);
        }
    }
    return result;
}

CMatrix CMatrix::operator+(double value) const
{
    CMatrix result(*this);
    for (int i = 0; i < n_rows; i++)
    {
        for (int j = 0; j < n_cols; j++)
        {
            result(i, j) += value;
        }
    }
    return result;
}

CMatrix CMatrix::operator-(double value) const
{
    CMatrix result(*this);
    for (int i = 0; i < n_rows; i++)
    {
        for (int j = 0; j < n_cols; j++)
        {
            result(i, j) -= value;
        }
    }
    return result;
}

CMatrix CMatrix::Transp() const
{
    CMatrix result(n_cols, n_rows);
    for (int i = 0; i < n_rows; i++)
    {
        for (int j = 0; j < n_cols; j++)
        {
            result(j, i) = array[i][j];
        }
    }
    return result;
}

CMatrix CMatrix::GetRow(int row) const
{
    return GetRow(row, 0, n_cols - 1);
}

CMatrix CMatrix::GetRow(int row, int first_column, int last_column) const
{
    if (row < 0 || row >= n_rows || first_column < 0 ||
        first_column > last_column || last_column >= n_cols)
    {
        throw out_of_range("Неверные границы строки матрицы.");
    }

    CMatrix result(1, last_column - first_column + 1);
    for (int column = first_column; column <= last_column; column++)
    {
        result(0, column - first_column) = array[row][column];
    }
    return result;
}

CMatrix CMatrix::GetCol(int column) const
{
    return GetCol(column, 0, n_rows - 1);
}

CMatrix CMatrix::GetCol(int column, int first_row, int last_row) const
{
    if (column < 0 || column >= n_cols || first_row < 0 ||
        first_row > last_row || last_row >= n_rows)
    {
        throw out_of_range("Неверные границы столбца матрицы.");
    }

    CMatrix result(last_row - first_row + 1, 1);
    for (int row = first_row; row <= last_row; row++)
    {
        result(row - first_row, 0) = array[row][column];
    }
    return result;
}

CMatrix& CMatrix::RedimMatrix(int rows_count, int columns_count)
{
    CMatrix resized(rows_count, columns_count);
    Swap(resized);
    return *this;
}

CMatrix& CMatrix::RedimData(int rows_count, int columns_count)
{
    CMatrix resized(rows_count, columns_count);
    int copied_rows = min(n_rows, rows_count);
    int copied_columns = min(n_cols, columns_count);

    for (int i = 0; i < copied_rows; i++)
    {
        for (int j = 0; j < copied_columns; j++)
        {
            resized(i, j) = array[i][j];
        }
    }

    Swap(resized);
    return *this;
}

CMatrix& CMatrix::RedimMatrix(int rows_count)
{
    return RedimMatrix(rows_count, 1);
}

CMatrix& CMatrix::RedimData(int rows_count)
{
    return RedimData(rows_count, 1);
}

double CMatrix::MaxElement() const
{
    double maximum = array[0][0];
    for (int i = 0; i < n_rows; i++)
    {
        for (int j = 0; j < n_cols; j++)
        {
            maximum = max(maximum, array[i][j]);
        }
    }
    return maximum;
}

double CMatrix::MinElement() const
{
    double minimum = array[0][0];
    for (int i = 0; i < n_rows; i++)
    {
        for (int j = 0; j < n_cols; j++)
        {
            minimum = min(minimum, array[i][j]);
        }
    }
    return minimum;
}
