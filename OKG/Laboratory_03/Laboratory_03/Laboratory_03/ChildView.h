#pragma once

#include "Plot2D.h"

enum class TestMode
{
    F1,
    F2,
    F3
};

class CChildView
{
private:
    HWND window_handle{};
    TestMode current_test{ TestMode::F1 };

    static LRESULT CALLBACK WindowProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
    LRESULT HandleMessage(UINT message, WPARAM wParam, LPARAM lParam);
    void SelectTest(TestMode test);
    void Paint();
    CRect GetPlotRect(const RECT& client_rect) const;
    void DrawFunction(HDC hdc, const RECT& client_rect, bool first_function);
    void DrawOctagon(HDC hdc, const RECT& client_rect);

public:
    bool Create(HINSTANCE instance, int show_command);
    int Run();
    double MyF1(double x);
    double MyF2(double x);
};
