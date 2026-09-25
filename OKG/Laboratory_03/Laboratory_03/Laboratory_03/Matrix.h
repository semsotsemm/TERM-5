#pragma once

#include <vector>

class CMatrix
{
private:
    int rows_count{};
    int columns_count{};
    std::vector<double> values;

public:
    CMatrix() = default;
    CMatrix(int rows, int columns);

    void RedimMatrix(int rows, int columns);
    int Rows() const;
    int Columns() const;
    double& operator()(int row, int column);
    double operator()(int row, int column) const;
};
