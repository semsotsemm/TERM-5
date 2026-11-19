#include "CPyramid.h"

#include <algorithm>
#include <cmath>
#include <numbers>

namespace
{
    constexpr double DEGREES_TO_RADIANS = std::numbers::pi / 180.0;
    constexpr double AMBIENT_LIGHT = 0.15;
    constexpr double DIFFUSE_LIGHT = 0.85;

    double DotProduct(const Point3D& first, const Point3D& second)
    {
        return first.x * second.x + first.y * second.y + first.z * second.z;
    }
}

CPyramid::CPyramid()
    : vertices{
        {4.0, 0.0, 0.0},
        {0.0, 0.0, 0.0},
        {0.0, 4.0, 0.0},
        {1.6, 0.0, 3.2},
        {0.0, 0.0, 3.2},
        {0.0, 1.6, 3.2}
    },
    faces{
        {{0, 1, 2, 0}, 3},
        {{3, 5, 4, 0}, 3},
        {{1, 0, 3, 4}, 4},
        {{2, 1, 4, 5}, 4},
        {{0, 2, 5, 3}, 4}
    },
    camera_position(3), light_position(3), light_color(RGB(255, 255, 255))
{
    camera_position(0) = 10.0;
    camera_position(1) = 45.0;
    camera_position(2) = 60.0;
    light_position(0) = 100.0;
    light_position(1) = 45.0;
    light_position(2) = 45.0;
}

double CPyramid::GetRadius() const
{
    return camera_position(0);
}

double CPyramid::GetPhi() const
{
    return camera_position(1);
}

double CPyramid::GetTheta() const
{
    return camera_position(2);
}

double CPyramid::GetLightRadius() const
{
    return light_position(0);
}

double CPyramid::GetLightPhi() const
{
    return light_position(1);
}

double CPyramid::GetLightTheta() const
{
    return light_position(2);
}

COLORREF CPyramid::GetLightColor() const
{
    return light_color;
}

bool CPyramid::SetAngles(CMatrix& position, double new_phi, double new_theta)
{
    if (!std::isfinite(new_phi) || !std::isfinite(new_theta) ||
        new_theta < 0.0 || new_theta > 180.0)
    {
        return false;
    }

    position(1) = std::fmod(new_phi, 360.0);
    if (position(1) < 0.0)
    {
        position(1) += 360.0;
    }
    position(2) = new_theta;
    return true;
}

bool CPyramid::SetCamera(double new_phi, double new_theta)
{
    return SetAngles(camera_position, new_phi, new_theta);
}

bool CPyramid::SetLight(double new_phi, double new_theta)
{
    return SetAngles(light_position, new_phi, new_theta);
}

void CPyramid::SetLightColor(COLORREF color)
{
    light_color = color;
}

Point3D CPyramid::SphericalToCartesian(const CMatrix& position)
{
    const double radius = position(0);
    const double phi = position(1) * DEGREES_TO_RADIANS;
    const double theta = position(2) * DEGREES_TO_RADIANS;

    return {
        radius * std::sin(theta) * std::cos(phi),
        radius * std::sin(theta) * std::sin(phi),
        radius * std::cos(theta)
    };
}

Point3D CPyramid::WorldToView(const Point3D& point, const CMatrix& position)
{
    const double phi = position(1) * DEGREES_TO_RADIANS;
    const double theta = position(2) * DEGREES_TO_RADIANS;
    const double sin_phi = std::sin(phi);
    const double cos_phi = std::cos(phi);
    const double sin_theta = std::sin(theta);
    const double cos_theta = std::cos(theta);

    return {
        -point.x * sin_phi + point.y * cos_phi,
        -point.x * cos_phi * cos_theta - point.y * sin_phi * cos_theta + point.z * sin_theta,
        position(0) - point.x * cos_phi * sin_theta - point.y * sin_phi * sin_theta - point.z * cos_theta
    };
}

Point3D CPyramid::GetFaceNormal(int face_index) const
{
    const Face& face = faces[face_index];
    const Point3D& first = vertices[face.vertices[0]];
    const Point3D& second = vertices[face.vertices[1]];
    const Point3D& third = vertices[face.vertices[2]];
    const Point3D first_edge = {second.x - first.x, second.y - first.y, second.z - first.z};
    const Point3D second_edge = {third.x - first.x, third.y - first.y, third.z - first.z};

    return {
        first_edge.y * second_edge.z - first_edge.z * second_edge.y,
        first_edge.z * second_edge.x - first_edge.x * second_edge.z,
        first_edge.x * second_edge.y - first_edge.y * second_edge.x
    };
}

Point3D CPyramid::GetFaceCenter(int face_index) const
{
    const Face& face = faces[face_index];
    Point3D center = {};
    for (int i = 0; i < face.count; i++)
    {
        const Point3D& point = vertices[face.vertices[i]];
        center.x += point.x;
        center.y += point.y;
        center.z += point.z;
    }
    center.x /= face.count;
    center.y /= face.count;
    center.z /= face.count;
    return center;
}

COLORREF CPyramid::GetFaceColor(int face_index, const Point3D& light, COLORREF color) const
{
    const Point3D normal = GetFaceNormal(face_index);
    const Point3D center = GetFaceCenter(face_index);
    const Point3D light_direction = {light.x - center.x, light.y - center.y, light.z - center.z};
    const double normal_length = std::sqrt(DotProduct(normal, normal));
    const double light_distance = std::sqrt(DotProduct(light_direction, light_direction));
    double diffuse = 0.0;

    if (normal_length > 0.0 && light_distance > 0.0)
    {
        diffuse = std::clamp(DotProduct(normal, light_direction) / (normal_length * light_distance), 0.0, 1.0);
    }

    const double intensity = AMBIENT_LIGHT + DIFFUSE_LIGHT * diffuse;
    return RGB(static_cast<int>(std::lround(GetRValue(color) * intensity)),
        static_cast<int>(std::lround(GetGValue(color) * intensity)),
        static_cast<int>(std::lround(GetBValue(color) * intensity)));
}

void CPyramid::Draw(CDC& dc, CRect& plot_rect)
{
    ColorDraw(dc, camera_position, light_position, plot_rect, light_color);
}

void CPyramid::ColorDraw(CDC& dc, CMatrix& PView, CMatrix& PLight, CRect& RW, COLORREF Color)
{
    if (RW.Width() <= 48 || RW.Height() <= 48)
    {
        return;
    }

    const Point3D axis_ends[3] = {{5.0, 0.0, 0.0}, {0.0, 5.0, 0.0}, {0.0, 0.0, 4.2}};
    Point3D view_points[9];
    for (int i = 0; i < 6; i++)
    {
        view_points[i] = WorldToView(vertices[i], PView);
    }
    for (int i = 0; i < 3; i++)
    {
        view_points[6 + i] = WorldToView(axis_ends[i], PView);
    }

    double min_x = view_points[0].x;
    double max_x = min_x;
    double min_y = view_points[0].y;
    double max_y = min_y;
    for (const Point3D& point : view_points)
    {
        min_x = std::min(min_x, point.x);
        max_x = std::max(max_x, point.x);
        min_y = std::min(min_y, point.y);
        max_y = std::max(max_y, point.y);
    }

    const double scale = std::min((RW.Width() - 48.0) / std::max(0.01, max_x - min_x),
        (RW.Height() - 48.0) / std::max(0.01, max_y - min_y));
    const double screen_center_x = (static_cast<double>(RW.left) + RW.right) / 2.0;
    const double screen_center_y = (static_cast<double>(RW.top) + RW.bottom) / 2.0;
    const double view_center_x = (min_x + max_x) / 2.0;
    const double view_center_y = (min_y + max_y) / 2.0;
    CPoint screen_points[9];
    for (int i = 0; i < 9; i++)
    {
        screen_points[i].x = std::lround(screen_center_x + scale * (view_points[i].x - view_center_x));
        screen_points[i].y = std::lround(screen_center_y - scale * (view_points[i].y - view_center_y));
    }

    const int saved_dc = dc.SaveDC();
    if (saved_dc == 0)
    {
        return;
    }

    dc.SetMapMode(MM_TEXT);
    dc.SetWindowOrg(0, 0);
    dc.SetViewportOrg(0, 0);
    dc.SetBkMode(TRANSPARENT);
    dc.SelectStockObject(DEFAULT_GUI_FONT);

    CPen axis_pen(PS_DOT, 1, RGB(150, 150, 150));
    dc.SelectObject(&axis_pen);
    for (int i = 0; i < 3; i++)
    {
        dc.MoveTo(screen_points[1]);
        dc.LineTo(screen_points[6 + i]);
    }

    CPen edge_pen(PS_SOLID, 1, RGB(30, 30, 30));
    dc.SelectObject(&edge_pen);
    dc.SelectStockObject(DC_BRUSH);
    const Point3D toward_camera = SphericalToCartesian(PView);
    const Point3D light = SphericalToCartesian(PLight);
    bool visible_vertices[6] = {};

    for (int i = 0; i < 5; i++)
    {
        if (DotProduct(GetFaceNormal(i), toward_camera) <= 1e-9)
        {
            continue;
        }

        const Face& face = faces[i];
        CPoint polygon[4];
        for (int j = 0; j < face.count; j++)
        {
            polygon[j] = screen_points[face.vertices[j]];
            visible_vertices[face.vertices[j]] = true;
        }

        ::SetDCBrushColor(dc.GetSafeHdc(), GetFaceColor(i, light, Color));
        dc.Polygon(polygon, face.count);
    }

    dc.SetTextColor(RGB(30, 30, 30));
    const wchar_t* vertex_labels[6] = {L"A", L"B", L"C", L"A1", L"B1", L"C1"};
    for (int i = 0; i < 6; i++)
    {
        if (visible_vertices[i])
        {
            const int offset_x = screen_points[i].x < screen_center_x ? -22 : 6;
            const int offset_y = screen_points[i].y < screen_center_y ? -18 : 3;
            const int label_x = screen_points[i].x + offset_x;
            const int label_y = screen_points[i].y + offset_y;
            const COLORREF background = dc.GetPixel(label_x + 2, label_y + 6);
            const int brightness = (299 * GetRValue(background) + 587 * GetGValue(background) +
                114 * GetBValue(background)) / 1000;
            dc.SetTextColor(brightness < 128 ? RGB(255, 255, 255) : RGB(30, 30, 30));
            dc.TextOutW(label_x, label_y, vertex_labels[i]);
        }
    }

    dc.SetTextColor(RGB(90, 90, 90));
    const wchar_t* axis_labels[3] = {L"X", L"Y", L"Z"};
    for (int i = 0; i < 3; i++)
    {
        const Point3D& end = view_points[6 + i];
        if (std::abs(end.x) + std::abs(end.y) > 1e-9)
        {
            dc.TextOutW(screen_points[6 + i].x + 6, screen_points[6 + i].y - 9, axis_labels[i]);
        }
    }
    dc.RestoreDC(saved_dc);
}
