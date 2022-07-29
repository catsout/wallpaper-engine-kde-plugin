#pragma once

#include "Particle.h"

#include <Eigen/Dense>
#include <cstdint>

namespace wallpaper
{

namespace ParticleModify
{

inline void Move(Particle& p, const Eigen::Vector3d& acc) noexcept {
    p.position = (p.position.cast<double>() + acc).cast<float>();
}
inline void Move(Particle& p, double x, double y, double z) noexcept { Move(p, { x, y, z }); }

inline void MoveTo(Particle& p, const Eigen::Vector3d& pos) noexcept {
    p.position = pos.cast<float>();
}
inline void MoveTo(Particle& p, double x, double y, double z) noexcept { MoveTo(p, { x, y, z }); }

inline void MoveToNegZ(Particle& p) noexcept { p.position.z() = -std::abs(p.position.z()); }

inline void MoveByTime(Particle& p, double t) noexcept { Move(p, p.velocity.cast<double>() * t); }

inline void MoveMultiply(Particle& p, const Eigen::Vector3d& para) noexcept {
    p.position = para.cwiseProduct(p.position.cast<double>()).cast<float>();
}
inline void MoveMultiply(Particle& p, double x, double y, double z) noexcept {
    MoveMultiply(p, { x, y, z });
}

inline void MoveApplySign(Particle& p, int32_t x, int32_t y, int32_t z) noexcept {
    if (x != 0) {
        p.position[0] = std::abs(p.position[0]) * (float)x;
    }
    if (y != 0) {
        p.position[1] = std::abs(p.position[1]) * (float)y;
    }
    if (z != 0) {
        p.position[2] = std::abs(p.position[2]) * (float)z;
    }
}
inline void SphereDirectOffset(Particle& p, const Eigen::Vector3d& base, double direct) noexcept {
    using namespace Eigen;
    Vector3d axis  = base.cross(p.position.cast<double>()).normalized();
    Affine3d trans = Affine3d::Identity();
    trans.prerotate(AngleAxis<double>(direct, axis));
    p.position = (trans * p.position.cast<double>()).cast<float>();
}

inline void RotatePos(Particle& p, double x, double y, double z) noexcept {
    using namespace Eigen;
    Affine3d trans = Affine3d::Identity();

    trans.prerotate(AngleAxis<double>(y, Vector3d::UnitY()));  // y
    trans.prerotate(AngleAxis<double>(x, Vector3d::UnitX()));  // x
    trans.prerotate(AngleAxis<double>(-z, Vector3d::UnitZ())); // z
    p.position = (trans * p.position.cast<double>()).cast<float>();
}

inline void ChangeLifetime(Particle& p, double l) noexcept { p.lifetime += l; }

inline double LifetimePos(const Particle& p) {
    if (p.lifetime < 0) return 1.0;
    return 1.0 - (p.lifetime / p.init.lifetime);
}

inline double LifetimePassed(const Particle& p) noexcept { return p.init.lifetime - p.lifetime; }

inline bool LifetimeOk(const Particle& p) noexcept { return p.lifetime > 0.0f; }

void ChangeRotation(Particle&, float x, float y, float z);

inline void ChangeColor(Particle& p, const Eigen::Vector3d& c) noexcept {
    p.color = (p.color.cast<double>() + c).cast<float>();
}
inline void ChangeColor(Particle& p, double r, double g, double b) { ChangeColor(p, { r, g, b }); }

inline void ChangeRotation(Particle& p, const Eigen::Vector3d& r) noexcept {
    p.rotation = (p.rotation.cast<double>() + r).cast<float>();
}
inline void ChangeRotation(Particle& p, double x, double y, double z) {
    ChangeRotation(p, { x, y, z });
}

inline void ChangeVelocity(Particle& p, const Eigen::Vector3d& v) noexcept {
    p.velocity = (p.velocity.cast<double>() + v).cast<float>();
}
inline void ChangeVelocity(Particle& p, double x, double y, double z) noexcept {
    ChangeVelocity(p, { x, y, z });
}
inline void Accelerate(Particle& p, const Eigen::Vector3d& acc, double t) noexcept {
    ChangeVelocity(p, acc * t);
}

inline void ChangeAngularVelocity(Particle& p, const Eigen::Vector3d& v) noexcept {
    p.angularVelocity = (p.angularVelocity.cast<double>() + v).cast<float>();
}
inline void ChangeAngularVelocity(Particle& p, double x, double y, double z) noexcept {
    ChangeAngularVelocity(p, { x, y, z });
}
inline void AngularAccelerate(Particle& p, const Eigen::Vector3d& acc, double t) noexcept {
    ChangeAngularVelocity(p, acc * t);
}

inline void Rotate(Particle& p, const Eigen::Vector3d& r) noexcept {
    p.rotation = (p.rotation.cast<double>() + r).cast<float>();
}
inline void Rotate(Particle& p, double x, double y, double z) noexcept { Rotate(p, { x, y, z }); }

inline void RotateByTime(Particle& p, double t) noexcept {
    Rotate(p, p.angularVelocity.cast<double>() * t);
}

inline void MutiplyAlpha(Particle& p, double a) { p.alpha *= a; }
inline void MutiplySize(Particle& p, double s) { p.size *= s; }

inline void MutiplyColor(Particle& p, const Eigen::Vector3d& c) {
    p.color = c.cwiseProduct(p.color.cast<double>()).cast<float>();
}
inline void MutiplyColor(Particle& p, double r, double g, double b) {
    MutiplyColor(p, { r, g, b });
}
inline void MutiplyVelocity(Particle& p, double m) { p.velocity *= m; }

inline void ChangeSize(Particle& p, double s) { p.size += s; }
inline void ChangeAlpha(Particle& p, double a) { p.alpha += a; }

inline void InitLifetime(Particle& p, float l) noexcept {
    p.lifetime      = l;
    p.init.lifetime = l;
}
inline void InitSize(Particle& p, double s) {
    p.size      = s;
    p.init.size = s;
}
inline void InitAlpha(Particle& p, double a) {
    p.alpha      = a;
    p.init.alpha = a;
}
inline void InitColor(Particle& p, double r, double g, double b) {
    Eigen::Vector3d c { r, g, b };
    p.color      = c.cast<float>();
    p.init.color = p.color;
}

inline void InitVelocity(Particle& p, const Eigen::Vector3d& v) { p.velocity = v.cast<float>(); }
inline void InitVelocity(Particle& p, double x, double y, double z) {
    InitVelocity(p, { x, y, z });
}

inline void MutiplyInitLifeTime(Particle& p, double m) {
    p.lifetime *= m;
    p.init.lifetime = p.lifetime;
}
inline void MutiplyInitAlpha(Particle& p, double m) {
    p.alpha *= m;
    p.init.alpha = p.alpha;
}
inline void MutiplyInitSize(Particle& p, double m) {
    p.size *= m;
    p.init.size = p.size;
}
inline void MutiplyInitColor(Particle& p, double r, double g, double b) {
    MutiplyColor(p, { r, g, b });
    p.init.color = p.color;
}

inline void Reset(Particle& p) {
    p.alpha = p.init.alpha;
    p.size  = p.init.size;
    p.color = p.init.color;
}

inline void MarkOld(Particle& p) { p.mark_new = false; }
inline bool IsNew(const Particle& p) { return p.mark_new; }

inline const Eigen::Vector3f& GetPos(const Particle& p) { return p.position; }
inline const Eigen::Vector3f& GetVelocity(const Particle& p) { return p.velocity; }
inline const Eigen::Vector3f& GetAngular(const Particle& p) { return p.rotation; }

}; // namespace ParticleModify
} // namespace wallpaper
