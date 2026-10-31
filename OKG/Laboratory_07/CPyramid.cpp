#include "CPyramid.h"

#include <algorithm>
#include <cmath>

const double PI = 3.14159265358979323846;

CPyramid::CPyramid()
    : vertices{
        {4.0, 0.0, 0.0},  // A
        {0.0, 0.0, 0.0},  // B: начало мировых координат
        {0.0, 4.0, 0.0},  // C
        {1.6, 0.0, 3.2},  // A1
        {0.0, 0.0, 3.2},  // B1
        {0.0, 1.6, 3.2}   // C1
    },
    faces{
        {{0, 1, 2, 0}, 3, RGB(245, 174, 92)},  // Нижняя грань
        {{3, 5, 4, 0}, 3, RGB(114, 174, 235)}, // Верхняя грань
        {{1, 0, 3, 4}, 4, RGB(238, 238, 238)},
        {{2, 1, 4, 5}, 4, RGB(222, 222, 222)},
        {{0, 2, 5, 3}, 4, RGB(248, 248, 248)}
    },
    radius(10.0), phi(315.0), theta(45.0)
{
}

double CPyramid::GetRadius() const
{
    return radius;
}

double CPyramid::GetPhi() const
{
    return phi;
}

double CPyramid::GetTheta() const
{
    return theta;
}

bool CPyramid::SetCamera(double new_phi, double new_theta)
{
    if (!std::isfinite(new_phi) || !std::isfinite(new_theta) ||
        new_theta < 0.0 || new_theta > 180.0)
    {
        return false;
    }

    phi = std::fmod(new_phi, 360.0);
    if (phi < 0.0)
    {
        phi += 360.0;
    }
    theta = new_theta;
    return true;
}

void CPyramid::ResetCamera()
{
    phi = 315.0;
    theta = 45.0;
}

Point3D CPyramid::GetCameraPosition() const
{
    const double phi_rad = phi * PI / 180.0;
    const double theta_rad = theta * PI / 180.0;

    return {
        radius * std::sin(theta_rad) * std::cos(phi_rad),
        radius * std::sin(theta_rad) * std::sin(phi_rad),
        radius * std::cos(theta_rad)
    };
}

Point3D CPyramid::WorldToView(const Point3D& point) const
{
    const double phi_rad = phi * PI / 180.0;
    const double theta_rad = theta * PI / 180.0;
    const double sin_phi = std::sin(phi_rad);
    const double cos_phi = std::cos(phi_rad);
    const double sin_theta = std::sin(theta_rad);
    const double cos_theta = std::cos(theta_rad);

    return {
        -point.x * sin_phi + point.y * cos_phi,
        -point.x * cos_phi * cos_theta - point.y * sin_phi * cos_theta + point.z * sin_theta,
        -point.x * cos_phi * sin_theta - point.y * sin_phi * sin_theta - point.z * cos_theta + radius
    };
}

Point3D CPyramid::GetFaceNormal(int face_index) const
{
    const Face& face = faces[face_index];
    const Point3D& a = vertices[face.vertices[0]];
    const Point3D& b = vertices[face.vertices[1]];
    const Point3D& c = vertices[face.vertices[2]];
    const Point3D ab = {b.x - a.x, b.y - a.y, b.z - a.z};
    const Point3D ac = {c.x - a.x, c.y - a.y, c.z - a.z};

    return {
        ab.y * ac.z - ab.z * ac.y,
        ab.z * ac.x - ab.x * ac.z,
        ab.x * ac.y - ab.y * ac.x
    };
}

bool CPyramid::IsFaceVisible(int face_index) const
{
    if (face_index < 0 || face_index >= 5)
    {
        return false;
    }

    const Point3D normal = GetFaceNormal(face_index);
    const Point3D direction = GetCameraPosition();

    return normal.x * direction.x + normal.y * direction.y + normal.z * direction.z > 1e-9;
}

void CPyramid::Draw(HDC hdc, const RECT& client_rect, bool remove_hidden_faces) const
{
    const int saved_dc = SaveDC(hdc);
    SetMapMode(hdc, MM_TEXT);
    SetBkMode(hdc, TRANSPARENT);
    SelectObject(hdc, GetStockObject(DEFAULT_GUI_FONT));

    Point3D view_points[9];
    for (int i = 0; i < 6; i++)
    {
        view_points[i] = WorldToView(vertices[i]);
    }

    const Point3D axis_ends[3] = {
        {5.0, 0.0, 0.0}, {0.0, 5.0, 0.0}, {0.0, 0.0, 4.2}
    };
    for (int i = 0; i < 3; i++)
    {
        view_points[6 + i] = WorldToView(axis_ends[i]);
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

    const double width = static_cast<double>(client_rect.right - client_rect.left);
    const double height = static_cast<double>(client_rect.bottom - client_rect.top);
    const double scale = std::max(1.0, std::min(
        (width - 140.0) / std::max(1.0, max_x - min_x),
        (height - 170.0) / std::max(1.0, max_y - min_y)
    ));
    const double center_x = client_rect.left + width / 2.0 - scale * (min_x + max_x) / 2.0;
    const double center_y = client_rect.top + 60.0 + (height - 60.0) / 2.0 + scale * (min_y + max_y) / 2.0;

    POINT screen_points[9];
    for (int i = 0; i < 9; i++)
    {
        screen_points[i].x = static_cast<LONG>(std::lround(center_x + scale * view_points[i].x));
        screen_points[i].y = static_cast<LONG>(std::lround(center_y - scale * view_points[i].y));
    }

    HPEN axis_pen = CreatePen(PS_DOT, 1, RGB(145, 145, 145));
    SelectObject(hdc, axis_pen);
    for (int i = 0; i < 3; i++)
    {
        MoveToEx(hdc, screen_points[1].x, screen_points[1].y, nullptr);
        LineTo(hdc, screen_points[6 + i].x, screen_points[6 + i].y);
    }

    HPEN edge_pen = CreatePen(PS_SOLID, 2, RGB(25, 25, 25));
    SelectObject(hdc, edge_pen);
    bool visible_vertices[6] = {};

    if (remove_hidden_faces)
    {
        for (int i = 0; i < 5; i++)
        {
            if (!IsFaceVisible(i))
            {
                continue;
            }

            const Face& face = faces[i];
            POINT polygon_points[4];
            for (int j = 0; j < face.count; j++)
            {
                polygon_points[j] = screen_points[face.vertices[j]];
                visible_vertices[face.vertices[j]] = true;
            }

            HBRUSH face_brush = CreateSolidBrush(face.color);
            HGDIOBJ old_brush = SelectObject(hdc, face_brush);
            Polygon(hdc, polygon_points, face.count);
            SelectObject(hdc, old_brush);
            DeleteObject(face_brush);
        }
    }
    else
    {
        const int edges[9][2] = {
            {0, 1}, {1, 2}, {2, 0},
            {3, 4}, {4, 5}, {5, 3},
            {0, 3}, {1, 4}, {2, 5}
        };
        for (const auto& edge : edges)
        {
            MoveToEx(hdc, screen_points[edge[0]].x, screen_points[edge[0]].y, nullptr);
            LineTo(hdc, screen_points[edge[1]].x, screen_points[edge[1]].y);
        }
        for (bool& visible : visible_vertices)
        {
            visible = true;
        }
    }

    SetTextColor(hdc, RGB(25, 25, 25));
    const wchar_t* labels[6] = {L"A", L"B", L"C", L"A1", L"B1", L"C1"};
    for (int i = 0; i < 6; i++)
    {
        if (visible_vertices[i])
        {
            TextOutW(hdc, screen_points[i].x + 7, screen_points[i].y + 4, labels[i], lstrlenW(labels[i]));
        }
    }
    const wchar_t* axis_labels[3] = {L"X", L"Y", L"Z"};
    SetTextColor(hdc, RGB(90, 90, 90));
    for (int i = 0; i < 3; i++)
    {
        TextOutW(hdc, screen_points[6 + i].x + 7, screen_points[6 + i].y - 9, axis_labels[i], 1);
    }

    RestoreDC(hdc, saved_dc);
    DeleteObject(axis_pen);
    DeleteObject(edge_pen);
}
