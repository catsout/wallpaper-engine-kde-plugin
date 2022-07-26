#pragma once
#include <functional>
#include <cassert>
#include <cmath>
#include <Eigen/Dense>

namespace wallpaper
{
namespace algorism
{
double CalculatePersperctiveDistance(double fov, double height);
double CalculatePersperctiveFov(double distence, double height);

constexpr uint32_t PowOfTwo(uint32_t x) {
    uint32_t pow2 { 8 };
    while (pow2 < x) pow2 *= 2;
    return pow2;
}

constexpr bool IsPowOfTwo(uint32_t x) { return (x > 1) && ((x & (x - 1)) == 0); }

inline std::array<double, 3> GenSphere(const std::function<float()>& random) noexcept {
    double x1, x2;
    double m;
    do {
        x1 = 1.0f - 2.0f * random();
        x2 = 1.0f - 2.0f * random();
        m  = x1 * x1 + x2 * x2;
    } while (m > 1.0f);
    return { 2.0f * x1 * std::sqrt(1.0f - m), 2.0f * x2 * std::sqrt(1.0f - m), 1.0f - 2.0f * m };
}
inline std::array<double, 3> GenSphereHalf(const std::function<float()>& random) noexcept {
    double z = 1.0f - 2.0f * random();
    double r = std::sqrt(1 - std::pow(z, 2));
    double d = 2 * M_PI * (1.0f - 2.0f * random());
    return { r * std::cos(d), r * std::sin(d), z };
}

inline double DragForce(double speed, double strength, double density) {
    // return -0.5f * speed*speed * strength * density;
    return -speed * strength * density;
}
inline Eigen::Vector3d DragForce(Eigen::Vector3d v, double strength, double density = 1.0f) {
    return v.normalized() * DragForce(v.norm(), strength, density);
}

constexpr double       lerp(double t, double a, double b) noexcept { return a + t * (b - a); }
double                 PerlinNoise(double x, double y, double z) noexcept;
inline Eigen::Vector3d PerlinNoiseVec3(Eigen::Vector3d p) noexcept {
    return Eigen::Vector3d { PerlinNoise(p[0], p[1], p[2]),
                             PerlinNoise(p[0] + 89.156f, p[1] + 33.431f, p[2] + 57.12f),
                             PerlinNoise(p[0] + 150.823f, p[1] + 120.132f, p[2] + 142.22f) };
}

inline Eigen::Vector3d CurlNoise(Eigen::Vector3d p) noexcept {
    using namespace Eigen;
    const float e = 1e-4f;
    Vector3d    dx(e, 0, 0), dy(0, e, 0), dz(0, 0, e);
    Vector3d    x0 = PerlinNoiseVec3(p - dx), x1 = PerlinNoiseVec3(p + dx),
             y0 = PerlinNoiseVec3(p - dy), y1 = PerlinNoiseVec3(p + dy),
             z0 = PerlinNoiseVec3(p - dz), z1 = PerlinNoiseVec3(p + dz);

    double x = y1.z() - y0.z() - z1.y() + z0.y(), y = z1.x() - z0.x() - x1.z() + x0.z(),
           z = x1.y() - x0.y() - y1.x() + y0.x();

    const double divisor = 1.0f / (2.0f * e);
    return (Vector3d(x, y, z) * divisor).normalized();
}
} // namespace algorism
} // namespace wallpaper
