#pragma once
#include <cstdint>
#include <vector>
#include <string>
#include <Eigen/Geometry>
#include "Utils/span.hpp"

namespace wallpaper
{

class WPPuppet {
public:
    enum class PlayMode { Loop, Mirror, Single };
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

    struct AnimationLayer {
        uint16_t id { 0 };
        float    rate { 1.0f };
        float    blend { 1.0f };
        bool     visible { true };
        double   cur_time { 0.0f };
    };

public:
    std::vector<Bone>      bones;
    std::vector<Animation> anims;

    Span<Eigen::Affine3f> genFrame(std::vector<AnimationLayer>&, double time);
    void                  prepared();

private:
    struct AnimationLayer_in {
        AnimationLayer*              layer;
        float                        blend;
        Animation*                   anim { nullptr };
        Animation::InterpolationInfo interp_info;
    };
    std::vector<Eigen::Affine3f>   m_final_affines;
    std::vector<AnimationLayer_in> m_layers_in;
};
} // namespace wallpaper
