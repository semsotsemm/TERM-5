#pragma once

#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>

struct Point3D
{
    double x;
    double y;
    double z;
};

class CPyramid
{
private:
    struct Face
    {
        int vertices[4];
        int count;
        COLORREF color;
    };

    Point3D vertices[6];
    Face faces[5];
    double radius;
    double phi;
    double theta;

    Point3D GetFaceNormal(int face_index) const;

public:
    CPyramid();

    double GetRadius() const;
    double GetPhi() const;
    double GetTheta() const;
    bool SetCamera(double new_phi, double new_theta);
    void ResetCamera();

    Point3D GetCameraPosition() const;
    Point3D WorldToView(const Point3D& point) const;
    bool IsFaceVisible(int face_index) const;
    void Draw(HDC hdc, const RECT& client_rect, bool remove_hidden_faces) const;
};
