#pragma once
#include <cstdint>
#include <vector>
#include <string>
#include <memory>
#include <span>
#include <Eigen/Geometry>

namespace wallpaper
{

class WPPuppetLayer;

class WPPuppet {
public:
    enum class PlayMode
    {
        Loop,
        Mirror,
        Single
    };
    struct Bone {
        Eigen::Affine3f transform { Eigen::Affine3f::Identity() };
        uint32_t        parent { 0xFFFFFFFFu };

        bool noParent() const { return parent == 0xFFFFFFFFu; }
        // prepared
        Eigen::Affine3f offset_trans { Eigen::Affine3f::Identity() };
        /*
        Eigen::Vector3f world_axis_x;
        Eigen::Vector3f world_axis_y;
        Eigen::Vector3f world_axis_z;
        */
    };
    struct BoneFrame {
        Eigen::Vector3f position;
        Eigen::Vector3f angle;
        Eigen::Vector3f scale;

        // prepared
        Eigen::Quaternionf quaternion;
    };
    struct Animation {
        uint32_t    id;
        float       fps;
        uint32_t    length;
        PlayMode    mode;
        std::string name;

        struct BoneFrames {
            std::vector<BoneFrame> frames;
        };
        std::vector<BoneFrames> bframes_array;

        // prepared
        float max_time;
        float frame_time;
        struct InterpolationInfo {
            uint32_t frame_a;
            uint32_t frame_b;
            double   t;
        };
        InterpolationInfo getInterpolationInfo(double* cur_time) const;
    };

public:
    std::vector<Bone>      bones;
    std::vector<Animation> anims;

    std::span<const Eigen::Affine3f> genFrame(WPPuppetLayer&, double time) noexcept;
    void                             prepared();

private:
    std::vector<Eigen::Affine3f> m_final_affines;
};

class WPPuppetLayer {
    friend class WPPuppet;

public:
    WPPuppetLayer();
    WPPuppetLayer(std::shared_ptr<WPPuppet>);
    ~WPPuppetLayer();

    bool hasPuppet() const { return (bool)m_puppet; };

    struct AnimationLayer {
        uint32_t id { 0 };
        float    rate { 1.0f };
        float    blend { 1.0f };
        bool     visible { true };
        double   cur_time { 0.0f };
    };

    void prepared(std::span<AnimationLayer>);

    std::span<const Eigen::Affine3f> genFrame(double time) noexcept;

    void updateInterpolation(double time) noexcept;

private:
    struct Layer {
        AnimationLayer                         anim_layer;
        float                                  blend;
        const WPPuppet::Animation*             anim { nullptr };
        WPPuppet::Animation::InterpolationInfo interp_info;

        operator bool() const noexcept { return anim != nullptr; };
    };

    double m_global_blend { 1.0f };

    std::vector<Layer>        m_layers;
    std::shared_ptr<WPPuppet> m_puppet;
};

} // namespace wallpaper
