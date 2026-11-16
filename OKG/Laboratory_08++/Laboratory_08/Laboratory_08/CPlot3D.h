#pragma once

#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>

#include <array>
#include <cstddef>
#include <vector>

struct Point3D
{
    double x;
    double y;
    double z;
};

class CPlot3D
{
public:
    using SurfaceFunction = double (*)(double x, double y);

    CPlot3D();

    double GetRadius() const;
    double GetPhi() const;
    double GetTheta() const;
    bool SetCamera(double new_phi, double new_theta);
    void ResetCamera();

    void SetSurface(SurfaceFunction function, double min_x, double max_x,
        double min_y, double max_y, double step_x, double step_y);
    void Draw(HDC hdc, const RECT& plot_rect) const;

private:
    struct SurfaceFace
    {
        std::array<Point3D, 6> vertices;
        int vertex_count;
    };

    struct ViewFrame
    {
        Point3D right;
        Point3D up;
        Point3D toward_camera;
    };

    struct ProjectedFace
    {
        std::array<Point3D, 6> vertices;
        int vertex_count;
        double average_depth;
        COLORREF color;
    };

    std::vector<SurfaceFace> surface_faces;
    double radius;
    double phi;
    double theta;

    static SurfaceFace BuildFace(const std::array<Point3D, 4>& corners, SurfaceFunction function);
    static Point3D FindBoundaryPoint(Point3D inside, Point3D outside, SurfaceFunction function);
    ViewFrame GetViewFrame() const;
    Point3D WorldToView(const Point3D& point, const ViewFrame& view_frame) const;
    std::vector<ProjectedFace> ProjectFaces(const ViewFrame& view_frame) const;
};
