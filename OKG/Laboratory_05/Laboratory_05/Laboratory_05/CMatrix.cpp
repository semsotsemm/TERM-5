#include "CMatrix.h"

#include <algorithm>
#include <stdexcept>
#include <utility>

using namespace std;

// Создание матрицы [количество строк, количество столбцов] -> новая матрица (n на m)
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

// Освобождение места, удаление матрицы [] -> void
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

// Замена матрицы на существующую [Существующая матрица] -> void
void CMatrix::Swap(CMatrix& matrix)
{
    swap(array, matrix.array);
    swap(n_rows, matrix.n_rows);
    swap(n_cols, matrix.n_cols);
}

// Проверка, не выходит ли переданный индекс за пределы матрицы [индекс строки, индекс столбца] -> throw при ошибке, иначе void
void CMatrix::CheckIndex(int row, int column) const
{
    if (row < 0 || row >= n_rows || column < 0 || column >= n_cols)
    {
        throw out_of_range("Индекс элемента выходит за размеры матрицы.");
    }
}

// Конструктор матрицы по умолчанию [] -> Матрица 1 на 1
CMatrix::CMatrix() : array(nullptr), n_rows(0), n_cols(0)
{
    Allocate(1, 1);
}

// Конструктор матрицы [Количество строк n, количество столбцов m] -> новая матрица n на m
CMatrix::CMatrix(int rows_count, int columns_count)
    : array(nullptr), n_rows(0), n_cols(0)
{
    Allocate(rows_count, columns_count);
}

// Конструктор матрицы-вектора [количество строк n] -> матрица-вектор n на 1 
CMatrix::CMatrix(int rows_count) : array(nullptr), n_rows(0), n_cols(0)
{
    Allocate(rows_count, 1);
}

// Конструктор матрицы, копирование существующей [существующая матрица] -> дубликат существующей матрицы 
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

// Деструктор матрицы, освобождает место
CMatrix::~CMatrix()
{
    Free();
}

// Перегрузка () — доступ к элементам временной матрицы при помощи ()
double& CMatrix::operator()(int row, int column)
{
    CheckIndex(row, column);
    return array[row][column];
}

// Перегрузка () — доступ к элементам постоянной матрицы при помощи ()
const double& CMatrix::operator()(int row, int column) const
{
    CheckIndex(row, column);
    return array[row][column];
}

// Перегрузка () — доступ к элементам временной матрицы при помощи ()
double& CMatrix::operator()(int row)
{
    if (n_cols != 1)
    {
        throw logic_error("Один индекс можно использовать только для вектора-столбца.");
    }
    return (*this)(row, 0);
}

// Перегрузка () — доступ к элементам постоянной матрицы при помощи ()
const double& CMatrix::operator()(int row) const
{
    if (n_cols != 1)
    {
        throw logic_error("Один индекс можно использовать только для вектора-столбца.");
    }
    return (*this)(row, 0);
}

// Замена знаков всех переменных на противоположные [] -> такая-же матрица, где все знаки обратные
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

// Перегрузка оператора присваивания [новая матрица] -> копия новой матрицы
CMatrix& CMatrix::operator=(const CMatrix& matrix)
{
    if (this == &matrix)
    {
        return *this;
    }

    CMatrix copy(matrix);
    Swap(copy);
    return *this;
}

// Перемножение двух матриц [вторая матрица] -> матричное произведение двух матриц
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

// Сложение двух матриц [вторая матрица] -> сумма двух матриц
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

// Вычитание двух матриц [вторая матрица] -> разность двух матриц
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

// Сложение матрицы со скаляром [скаляр] -> матрица, каждый элемент которой большое исходного на скаляр
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

// Вычитание из матрицы скаляра [скаляр] -> матрица, каждый элемент которой меньше исходного на скаляр
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

// Транспонирование матрицы (замена строк на столбцы и наоборот) [] -> транспонирование матрицы
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

// Копирование всей строки матрицы в отдельный объект вектор-матрицы [номер строки] -> строка в виде вектор-матрицы
CMatrix CMatrix::GetRow(int row) const
{
    return GetRow(row, 0, n_cols - 1);
}

// Копирование определенной строки матрицы в отдельный объект вектор-матрицы [номер строки, начальный столбец, конечный столбец] -> строка в виде вектор-матрицы с заданным началом и концом
CMatrix CMatrix::GetRow(int row, int first_column, int last_column) const
{
    if (row < 0 || row >= n_rows || first_column < 0 || first_column > last_column || last_column >= n_cols)
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

// Копирование всего столбца матрицы в отдельный объект вектор-матрицы [номер столбеца] -> столбец в виде вектор-матрицы
CMatrix CMatrix::GetCol(int column) const
{
    return GetCol(column, 0, n_rows - 1);
}

// Копирование определенного столбца матрицы в отдельный объект вектор-матрицы [номер столбца, начальная строка, конечная строка] -> столбец в виде вектор-матрицы с заданным началом и концом
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

// Изменяет размеры матрицы, исходные данные удаляются [количество строк, количество столбцов] -> новая матрица, размером n на m
CMatrix& CMatrix::RedimMatrix(int rows_count, int columns_count)
{
    CMatrix resized(rows_count, columns_count);
    Swap(resized);
    return *this;
}

// Изменяет размеры матрицы, исходные данные сохраняются [количество строк, количество столбцов] -> новая матрица, размером n на m
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

// Пересоздаем матрицу в виде вектор-матрицы (данные удаляются) [количество строк] -> вектор матрица с 1 столбцом и n строк
CMatrix& CMatrix::RedimMatrix(int rows_count)
{
    return RedimMatrix(rows_count, 1);
}

// Пересоздаем матрицу в виде вектор-матрицы (данные сохраняются) [количество строк] -> вектор матрица с 1 столбцом и n строк
CMatrix& CMatrix::RedimData(int rows_count)
{
    return RedimData(rows_count, 1);
}

// Получение максимального элемента матрицы [] -> максимальный элемент матрицы
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

// Получение минимального элемента матрицы [] -> минимальный элемент матрицы
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