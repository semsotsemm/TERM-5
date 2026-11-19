#pragma once

#ifndef NOMINMAX
#define NOMINMAX
#endif

#include <afxwin.h>
#include "CMatrix.h"

struct Point3D
{
    double x;
    double y;
    double z;
};

class CPyramid
{
public:
    CPyramid();

    double GetRadius() const;
    double GetPhi() const;
    double GetTheta() const;
    double GetLightRadius() const;
    double GetLightPhi() const;
    double GetLightTheta() const;
    COLORREF GetLightColor() const;

    bool SetCamera(double new_phi, double new_theta);
    bool SetLight(double new_phi, double new_theta);
    void SetLightColor(COLORREF color);
    void Draw(CDC& dc, CRect& plot_rect);
    void ColorDraw(CDC& dc, CMatrix& PView, CMatrix& PLight, CRect& RW, COLORREF Color);

private:
    struct Face
    {
        int vertices[4];
        int count;
    };

    Point3D vertices[6];
    Face faces[5];
    CMatrix camera_position;
    CMatrix light_position;
    COLORREF light_color;

    static bool SetAngles(CMatrix& position, double new_phi, double new_theta);
    static Point3D SphericalToCartesian(const CMatrix& position);
    static Point3D WorldToView(const Point3D& point, const CMatrix& position);
    Point3D GetFaceNormal(int face_index) const;
    Point3D GetFaceCenter(int face_index) const;
    COLORREF GetFaceColor(int face_index, const Point3D& light, COLORREF color) const;
};
