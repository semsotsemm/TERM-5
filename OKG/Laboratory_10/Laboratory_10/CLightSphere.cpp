#include "CLightSphere.h"

#include <algorithm>
#include <cmath>
#include <numbers>
#include <vector>

namespace
{
    constexpr double DEGREES_TO_RADIANS = std::numbers::pi / 180.0;
    constexpr double AMBIENT_LIGHT = 0.12;
    constexpr double SPECULAR_POWER = 24.0;
    constexpr int MAX_SCREEN_RADIUS = 420;

    struct Point3D
    {
        double x;
        double y;
        double z;
    };

    double DotProduct(const Point3D& first, const Point3D& second)
    {
        return first.x * second.x + first.y * second.y + first.z * second.z;
    }

    Point3D DirectionTo(const Point3D& point, const Point3D& target)
    {
        const Point3D direction = {target.x - point.x, target.y - point.y, target.z - point.z};
        const double length = std::sqrt(DotProduct(direction, direction));
        if (length == 0.0)
        {
            return {};
        }

        return {direction.x / length, direction.y / length, direction.z / length};
    }

    Point3D SphericalToCartesian(double radius, double sin_phi, double cos_phi,
        double sin_theta, double cos_theta)
    {
        return {
            radius * sin_theta * cos_phi,
            radius * sin_theta * sin_phi,
            radius * cos_theta
        };
    }

    Point3D SphericalToCartesian(const CMatrix& position)
    {
        const double phi = position(1) * DEGREES_TO_RADIANS;
        const double theta = position(2) * DEGREES_TO_RADIANS;
        return SphericalToCartesian(position(0), std::sin(phi), std::cos(phi),
            std::sin(theta), std::cos(theta));
    }

    DWORD GetPixelColor(COLORREF color, double intensity)
    {
        const DWORD red = static_cast<DWORD>(std::lround(intensity * GetRValue(color)));
        const DWORD green = static_cast<DWORD>(std::lround(intensity * GetGValue(color)));
        const DWORD blue = static_cast<DWORD>(std::lround(intensity * GetBValue(color)));
        return (red << 16) | (green << 8) | blue;
    }

    double GetLightIntensity(const Point3D& point, const Point3D& normal,
        const Point3D& light, const Point3D& camera, int model_index)
    {
        const Point3D light_direction = DirectionTo(point, light);
        const double diffuse = std::clamp(DotProduct(normal, light_direction), 0.0, 1.0);
        double reflection = diffuse;

        if (model_index == 1 && diffuse > 0.0)
        {
            const Point3D reflected_light = {
                2.0 * diffuse * normal.x - light_direction.x,
                2.0 * diffuse * normal.y - light_direction.y,
                2.0 * diffuse * normal.z - light_direction.z
            };
            const Point3D camera_direction = DirectionTo(point, camera);
            const double specular = std::clamp(DotProduct(reflected_light, camera_direction), 0.0, 1.0);
            reflection = std::pow(specular, SPECULAR_POWER);
        }

        return AMBIENT_LIGHT + (1.0 - AMBIENT_LIGHT) * reflection;
    }
}

CLightSphere::CLightSphere()
    : radius(2.0), camera_position(3), light_position(3), light_color(RGB(255, 255, 0))
{
    camera_position(0) = 15.0;
    camera_position(1) = 30.0;
    camera_position(2) = 60.0;
    light_position(0) = 10.0;
    light_position(1) = 45.0;
    light_position(2) = 45.0;
}

double CLightSphere::GetCameraRadius() const
{
    return camera_position(0);
}

double CLightSphere::GetCameraPhi() const
{
    return camera_position(1);
}

double CLightSphere::GetCameraTheta() const
{
    return camera_position(2);
}

double CLightSphere::GetLightRadius() const
{
    return light_position(0);
}

double CLightSphere::GetLightPhi() const
{
    return light_position(1);
}

double CLightSphere::GetLightTheta() const
{
    return light_position(2);
}

COLORREF CLightSphere::GetLightColor() const
{
    return light_color;
}

bool CLightSphere::SetAngles(CMatrix& position, double new_phi, double new_theta)
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

bool CLightSphere::SetCamera(double new_phi, double new_theta)
{
    return SetAngles(camera_position, new_phi, new_theta);
}

bool CLightSphere::SetLight(double new_phi, double new_theta)
{
    return SetAngles(light_position, new_phi, new_theta);
}

void CLightSphere::SetLightColor(COLORREF color)
{
    light_color = color;
}

void CLightSphere::Draw(CDC& dc, const CRect& plot_rect, int model_index)
{
    DrawLightSphere(dc, radius, camera_position, light_position, plot_rect, light_color, model_index);
}

void DrawLightSphere(CDC& dc, double radius, CMatrix& camera_position,
    CMatrix& light_position, CRect plot_rect, COLORREF light_color, int model_index)
{
    const int plot_size = std::min(plot_rect.Width(), plot_rect.Height());
    if (plot_size < 4 || radius <= 0.0 || (model_index != 0 && model_index != 1))
    {
        return;
    }

    const int screen_radius = std::min(MAX_SCREEN_RADIUS, static_cast<int>(plot_size * 0.43));
    const int bitmap_size = 2 * screen_radius + 1;
    const size_t pixel_count = static_cast<size_t>(bitmap_size) * bitmap_size;
    std::vector<DWORD> pixels(pixel_count, GetPixelColor(GetSysColor(COLOR_WINDOW), 1.0));
    std::vector<float> depth_buffer(pixel_count, -1.0f);

    const Point3D light = SphericalToCartesian(light_position);
    const Point3D camera = SphericalToCartesian(camera_position);
    const Point3D camera_axis = DirectionTo({}, camera);
    const double camera_phi = camera_position(1) * DEGREES_TO_RADIANS;
    const double camera_theta = camera_position(2) * DEGREES_TO_RADIANS;
    const double sin_phi = std::sin(camera_phi);
    const double cos_phi = std::cos(camera_phi);
    const double sin_theta = std::sin(camera_theta);
    const double cos_theta = std::cos(camera_theta);
    const Point3D screen_right = {-sin_phi, cos_phi, 0.0};
    const Point3D screen_up = {-cos_phi * cos_theta, -sin_phi * cos_theta, sin_theta};

    const double max_step = std::min(DEGREES_TO_RADIANS, 0.52 / screen_radius);
    const int phi_count = static_cast<int>(std::ceil(2.0 * std::numbers::pi / max_step));
    const int theta_count = static_cast<int>(std::ceil(std::numbers::pi / max_step));
    const double phi_step = 2.0 * std::numbers::pi / phi_count;
    const double theta_step = std::numbers::pi / theta_count;
    std::vector<double> phi_sines(phi_count);
    std::vector<double> phi_cosines(phi_count);
    for (int phi_index = 0; phi_index < phi_count; phi_index++)
    {
        const double phi = phi_index * phi_step;
        phi_sines[phi_index] = std::sin(phi);
        phi_cosines[phi_index] = std::cos(phi);
    }

    for (int theta_index = 0; theta_index <= theta_count; theta_index++)
    {
        const double theta = theta_index * theta_step;
        const double theta_sine = std::sin(theta);
        const double theta_cosine = std::cos(theta);
        const bool is_pole = theta_index == 0 || theta_index == theta_count;
        const int ring_size = is_pole ? 1 : phi_count;
        for (int phi_index = 0; phi_index < ring_size; phi_index++)
        {
            const Point3D normal = SphericalToCartesian(1.0, phi_sines[phi_index], phi_cosines[phi_index],
                theta_sine, theta_cosine);
            const double depth = DotProduct(normal, camera_axis);
            if (depth < 0.0)
            {
                continue;
            }

            const int x = screen_radius + static_cast<int>(std::lround(DotProduct(normal, screen_right) * screen_radius));
            const int y = screen_radius - static_cast<int>(std::lround(DotProduct(normal, screen_up) * screen_radius));
            const size_t pixel = static_cast<size_t>(y) * bitmap_size + x;
            if (depth <= depth_buffer[pixel])
            {
                continue;
            }

            const Point3D point = {normal.x * radius, normal.y * radius, normal.z * radius};
            const double intensity = GetLightIntensity(point, normal, light, camera, model_index);
            pixels[pixel] = GetPixelColor(light_color, intensity);
            depth_buffer[pixel] = static_cast<float>(depth);
        }
    }

    BITMAPINFO bitmap_info = {};
    bitmap_info.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
    bitmap_info.bmiHeader.biWidth = bitmap_size;
    bitmap_info.bmiHeader.biHeight = -bitmap_size;
    bitmap_info.bmiHeader.biPlanes = 1;
    bitmap_info.bmiHeader.biBitCount = 32;
    bitmap_info.bmiHeader.biCompression = BI_RGB;

    const CPoint center = plot_rect.CenterPoint();
    SetDIBitsToDevice(dc.GetSafeHdc(), center.x - screen_radius, center.y - screen_radius,
        bitmap_size, bitmap_size, 0, 0, 0, bitmap_size, pixels.data(), &bitmap_info, DIB_RGB_COLORS);
}
