#pragma once

class CMatrix
{
private:
    double** array; 
    int n_rows;     
    int n_cols;     

    void Allocate(int rows_count, int columns_count);
    void Free();
    void Swap(CMatrix& matrix);
    void CheckIndex(int row, int column) const;

public:
    CMatrix();
    CMatrix(int rows_count, int columns_count);
    explicit CMatrix(int rows_count);
    CMatrix(const CMatrix& matrix);
    ~CMatrix();

    double& operator()(int row, int column);
    const double& operator()(int row, int column) const;
    double& operator()(int row);
    const double& operator()(int row) const;

    CMatrix operator-() const;
    CMatrix& operator=(const CMatrix& matrix);
    CMatrix operator*(const CMatrix& matrix) const;
    CMatrix operator+(const CMatrix& matrix) const;
    CMatrix operator-(const CMatrix& matrix) const;
    CMatrix operator+(double value) const;
    CMatrix operator-(double value) const;

    int rows() const { return n_rows; }
    int cols() const { return n_cols; }

    CMatrix Transp() const;
    CMatrix GetRow(int row) const;
    CMatrix GetRow(int row, int first_column, int last_column) const;
    CMatrix GetCol(int column) const;
    CMatrix GetCol(int column, int first_row, int last_row) const;

    CMatrix& RedimMatrix(int rows_count, int columns_count);
    CMatrix& RedimData(int rows_count, int columns_count);
    CMatrix& RedimMatrix(int rows_count);
    CMatrix& RedimData(int rows_count);

    double MaxElement() const;
    double MinElement() const;
};
