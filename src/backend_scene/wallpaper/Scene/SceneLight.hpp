#pragma once
#include <cstdint>
#include <vector>
#include <memory>
#include <Eigen/Dense>
#include "SceneNode.h"

namespace wallpaper
{

class SceneLight {
public:
    SceneLight(Eigen::Vector3f color, float radius, float intensity)
        : m_color(color), m_radius(radius), m_intensity(intensity) {
        m_premultiplied_color = m_color * m_intensity * m_radius * m_radius;
    }
    ~SceneLight() = default;

    Eigen::Vector3f color() const { return m_color; }
    float           radius() const { return m_radius; }
    SceneNode*      node() const { return m_node.get(); }

    Eigen::Vector3f premultipliedColor() const { return m_premultiplied_color; }

    void setNode(std::shared_ptr<SceneNode> node) { m_node = node; }

private:
    Eigen::Vector3f m_color { Eigen::Vector3f::Zero() };
    float           m_radius { 0.0f };
    float           m_intensity { 1.0f };

    Eigen::Vector3f            m_premultiplied_color { Eigen::Vector3f::Zero() };
    std::shared_ptr<SceneNode> m_node { nullptr };
};
} // namespace wallpaper
