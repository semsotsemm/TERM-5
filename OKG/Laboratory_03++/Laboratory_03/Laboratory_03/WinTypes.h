#pragma once

#ifndef NOMINMAX
#define NOMINMAX
#endif

#include <windows.h>

struct CRect
{
    int left{};
    int top{};
    int right{};
    int bottom{};

    CRect() = default;
    CRect(int left_value, int top_value, int right_value, int bottom_value)
        : left(left_value), top(top_value), right(right_value), bottom(bottom_value)
    {
    }

    int Width() const
    {
        return right - left;
    }

    int Height() const
    {
        return bottom - top;
    }
};

struct CRectD
{
    double left{};
    double top{};
    double right{};
    double bottom{};

    CRectD() = default;
    CRectD(double left_value, double top_value, double right_value, double bottom_value)
        : left(left_value), top(top_value), right(right_value), bottom(bottom_value)
    {
    }
};

class CDC
{
private:
    HDC hdc{};

public:
    explicit CDC(HDC device_context) : hdc(device_context)
    {
    }

    HDC GetSafeHdc() const
    {
        return hdc;
    }
};
