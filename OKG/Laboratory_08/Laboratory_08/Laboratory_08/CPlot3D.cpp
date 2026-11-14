#include "CPlot3D.h"

#include <algorithm>
#include <cmath>
#include <limits>
#include <numbers>

namespace
{
    constexpr double DEFAULT_RADIUS = 10.0;
    constexpr double DEFAULT_PHI = 45.0;
    constexpr double DEFAULT_THETA = 45.0;
    constexpr double DEGREES_TO_RADIANS = std::numbers::pi / 180.0;

    double DotProduct(const Point3D& first, const Point3D& second)
    {
        return first.x * second.x + first.y * second.y + first.z * second.z;
    }

    COLORREF GetFaceColor(const Point3D& first, const Point3D& second, const Point3D& third)
    {
        const Point3D first_edge = {second.x - first.x, second.y - first.y, second.z - first.z};
        const Point3D second_edge = {third.x - first.x, third.y - first.y, third.z - first.z};
        const Point3D normal = {
            first_edge.y * second_edge.z - first_edge.z * second_edge.y,
            first_edge.z * second_edge.x - first_edge.x * second_edge.z,
            first_edge.x * second_edge.y - first_edge.y * second_edge.x
        };
        const double normal_length = std::sqrt(DotProduct(normal, normal));
        const Point3D light_direction = {0.36, -0.48, 0.8};
        const double light = normal_length > 0.0
            ? std::min(1.0, std::abs(DotProduct(normal, light_direction)) / normal_length)
            : 0.5;

        return RGB(93 + static_cast<int>(103 * light),
            146 + static_cast<int>(74 * light), 199 + static_cast<int>(44 * light));
    }
}

CPlot3D::CPlot3D()
    : radius(DEFAULT_RADIUS), phi(DEFAULT_PHI), theta(DEFAULT_THETA)
{
}

double CPlot3D::GetRadius() const
{
    return radius;
}

double CPlot3D::GetPhi() const
{
    return phi;
}

double CPlot3D::GetTheta() const
{
    return theta;
}

bool CPlot3D::SetCamera(double new_phi, double new_theta)
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

void CPlot3D::ResetCamera()
{
    phi = DEFAULT_PHI;
    theta = DEFAULT_THETA;
}

void CPlot3D::SetSurface(SurfaceFunction function, double min_x, double max_x,
    double min_y, double max_y, double step_x, double step_y)
{
    surface_faces.clear();

    const int column_count = static_cast<int>(std::ceil((max_x - min_x) / step_x));
    const int row_count = static_cast<int>(std::ceil((max_y - min_y) / step_y));
    surface_faces.reserve(static_cast<std::size_t>(column_count) * row_count);

    for (int row = 0; row < row_count; row++)
    {
        const double bottom_y = min_y + row * step_y;
        const double top_y = std::min(max_y, min_y + (row + 1) * step_y);

        for (int column = 0; column < column_count; column++)
        {
            const double left_x = min_x + column * step_x;
            const double right_x = std::min(max_x, min_x + (column + 1) * step_x);
            const std::array<Point3D, 4> corners = {{
                {left_x, bottom_y, function(left_x, bottom_y)},
                {right_x, bottom_y, function(right_x, bottom_y)},
                {right_x, top_y, function(right_x, top_y)},
                {left_x, top_y, function(left_x, top_y)}
            }};
            const SurfaceFace face = BuildFace(corners, function);

            if (face.vertex_count >= 3)
            {
                surface_faces.push_back(face);
            }
        }
    }
}

CPlot3D::SurfaceFace CPlot3D::BuildFace(const std::array<Point3D, 4>& corners, SurfaceFunction function)
{
    SurfaceFace face = {};

    for (std::size_t corner_index = 0; corner_index < corners.size(); corner_index++)
    {
        const Point3D& current_corner = corners[corner_index];
        const Point3D& next_corner = corners[(corner_index + 1) % corners.size()];
        const bool current_inside = std::isfinite(current_corner.z);
        const bool next_inside = std::isfinite(next_corner.z);

        if (current_inside)
        {
            face.vertices[face.vertex_count++] = current_corner;
        }

        if (current_inside != next_inside)
        {
            const Point3D boundary = current_inside
                ? FindBoundaryPoint(current_corner, next_corner, function)
                : FindBoundaryPoint(next_corner, current_corner, function);
            const Point3D& inside_corner = current_inside ? current_corner : next_corner;

            if (boundary.x != inside_corner.x || boundary.y != inside_corner.y)
            {
                face.vertices[face.vertex_count++] = boundary;
            }
        }
    }

    return face;
}

Point3D CPlot3D::FindBoundaryPoint(Point3D inside, Point3D outside, SurfaceFunction function)
{
    for (int iteration = 0; iteration < 24; iteration++)
    {
        Point3D middle = {(inside.x + outside.x) / 2.0, (inside.y + outside.y) / 2.0, 0.0};
        middle.z = function(middle.x, middle.y);

        if (std::isfinite(middle.z))
        {
            inside = middle;
        }
        else
        {
            outside = middle;
        }
    }

    return inside;
}

CPlot3D::ViewFrame CPlot3D::GetViewFrame() const
{
    const double phi_radians = phi * DEGREES_TO_RADIANS;
    const double theta_radians = theta * DEGREES_TO_RADIANS;
    const double sin_phi = std::sin(phi_radians);
    const double cos_phi = std::cos(phi_radians);
    const double sin_theta = std::sin(theta_radians);
    const double cos_theta = std::cos(theta_radians);

    return {
        {-sin_phi, cos_phi, 0.0},
        {-cos_phi * cos_theta, -sin_phi * cos_theta, sin_theta},
        {cos_phi * sin_theta, sin_phi * sin_theta, cos_theta}
    };
}

Point3D CPlot3D::WorldToView(const Point3D& point, const ViewFrame& view_frame) const
{
    return {
        DotProduct(point, view_frame.right),
        DotProduct(point, view_frame.up),
        radius - DotProduct(point, view_frame.toward_camera)
    };
}

std::vector<CPlot3D::ProjectedFace> CPlot3D::ProjectFaces(const ViewFrame& view_frame) const
{
    std::vector<ProjectedFace> projected_faces;
    projected_faces.reserve(surface_faces.size());

    for (const SurfaceFace& face : surface_faces)
    {
        ProjectedFace projected_face = {};
        projected_face.vertex_count = face.vertex_count;
        projected_face.color = GetFaceColor(face.vertices[0], face.vertices[1], face.vertices[2]);

        for (int vertex_index = 0; vertex_index < face.vertex_count; vertex_index++)
        {
            projected_face.vertices[vertex_index] = WorldToView(face.vertices[vertex_index], view_frame);
            projected_face.average_depth += projected_face.vertices[vertex_index].z;
        }

        projected_face.average_depth /= face.vertex_count;
        projected_faces.push_back(projected_face);
    }

    std::stable_sort(projected_faces.begin(), projected_faces.end(),
        [](const ProjectedFace& first, const ProjectedFace& second)
        {
            return first.average_depth > second.average_depth;
        });

    return projected_faces;
}

void CPlot3D::Draw(HDC hdc, const RECT& plot_rect) const
{
    if (surface_faces.empty() || plot_rect.right <= plot_rect.left || plot_rect.bottom <= plot_rect.top)
    {
        return;
    }

    const std::vector<ProjectedFace> projected_faces = ProjectFaces(GetViewFrame());
    double min_x = std::numeric_limits<double>::max();
    double max_x = std::numeric_limits<double>::lowest();
    double min_y = min_x;
    double max_y = max_x;

    for (const ProjectedFace& face : projected_faces)
    {
        for (int vertex_index = 0; vertex_index < face.vertex_count; vertex_index++)
        {
            const Point3D& point = face.vertices[vertex_index];
            min_x = std::min(min_x, point.x);
            max_x = std::max(max_x, point.x);
            min_y = std::min(min_y, point.y);
            max_y = std::max(max_y, point.y);
        }
    }

    const double plot_width = static_cast<double>(plot_rect.right - plot_rect.left);
    const double plot_height = static_cast<double>(plot_rect.bottom - plot_rect.top);
    const double scale = std::min(plot_width / std::max(0.01, max_x - min_x),
        plot_height / std::max(0.01, max_y - min_y));
    const double screen_center_x = plot_rect.left + plot_width / 2.0;
    const double screen_center_y = plot_rect.top + plot_height / 2.0;
    const double view_center_x = (min_x + max_x) / 2.0;
    const double view_center_y = (min_y + max_y) / 2.0;

    const int saved_dc = SaveDC(hdc);
    if (saved_dc == 0)
    {
        return;
    }

    SetMapMode(hdc, MM_TEXT);
    SelectObject(hdc, GetStockObject(DC_PEN));
    SelectObject(hdc, GetStockObject(DC_BRUSH));
    SetDCPenColor(hdc, RGB(64, 102, 139));

    for (const ProjectedFace& face : projected_faces)
    {
        POINT polygon[6];
        for (int vertex_index = 0; vertex_index < face.vertex_count; vertex_index++)
        {
            const Point3D& point = face.vertices[vertex_index];
            polygon[vertex_index] = {
                std::lround(screen_center_x + scale * (point.x - view_center_x)),
                std::lround(screen_center_y - scale * (point.y - view_center_y))
            };
        }

        SetDCBrushColor(hdc, face.color);
        Polygon(hdc, polygon, face.vertex_count);
    }

    RestoreDC(hdc, saved_dc);
}
