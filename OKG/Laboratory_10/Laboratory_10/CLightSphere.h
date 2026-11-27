#pragma once

#ifndef NOMINMAX
#define NOMINMAX
#endif

#include <afxwin.h>
#include "CMatrix.h"

class CLightSphere
{
public:
    CLightSphere();

    double GetCameraRadius() const;
    double GetCameraPhi() const;
    double GetCameraTheta() const;
    double GetLightRadius() const;
    double GetLightPhi() const;
    double GetLightTheta() const;
    COLORREF GetLightColor() const;

    bool SetCamera(double new_phi, double new_theta);
    bool SetLight(double new_phi, double new_theta);
    void SetLightColor(COLORREF color);
    void Draw(CDC& dc, const CRect& plot_rect, int model_index);

private:
    double radius;
    CMatrix camera_position;
    CMatrix light_position;
    COLORREF light_color;

    static bool SetAngles(CMatrix& position, double new_phi, double new_theta);
};

void DrawLightSphere(CDC& dc, double radius, CMatrix& camera_position,
    CMatrix& light_position, CRect plot_rect, COLORREF light_color, int model_index);
