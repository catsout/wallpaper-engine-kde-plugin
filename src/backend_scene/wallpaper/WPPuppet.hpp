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
    enum class PlayMode {
        Loop, Mirror, Single
    };
    struct Bone {
        Eigen::Affine3f transform { Eigen::Affine3f::Identity() };
        uint32_t parent {0xFFFFFFFFu};

        bool noParent() const { return parent == 0xFFFFFFFFu; }
        // prepared
        Eigen::Affine3f offset_trans { Eigen::Affine3f::Identity() };
        Eigen::Vector3f world_axis_x;
        Eigen::Vector3f world_axis_y;
        Eigen::Vector3f world_axis_z;
    };
    struct BoneFrame {
        Eigen::Vector3f position;
        Eigen::Vector3f angle;
        Eigen::Vector3f scale;
    };
    struct Animation {
        uint32_t id;
        float fps;
        uint32_t length;
        PlayMode mode;
        std::string name;

        struct BoneFrames { std::vector<BoneFrame> frames; };
        std::vector<BoneFrames> bframes_array;
    };

    struct AnimationLayer {
        uint16_t id {0};
        float rate {1.0f};
        float blend {1.0f};
        bool visible {true};

        uint resolveModeIndex(uint i, int step, const Animation& anim) {
            uint res {0};
            if(anim.mode == PlayMode::Mirror) {
                if(m_mirror_b) {
                    res = std::abs((int)i-step) % anim.length;
                    if(i < step) {
                        m_mirror_b = false;
                    }
                } else {
                    res = i+step;
                    if(i+step > anim.length) {
                        res = anim.length - (res % anim.length);
                        m_mirror_b = true;
                    }
                }
            }
            else {
                res = (i+step) % anim.length;
            }
            return res;
        }

        uint nextFrame(double time, const Animation& anim) {
            time += m_remain_time;
            double frame_time = (1.0f / anim.fps);
            uint n = time / frame_time;

            m_cur_frame = resolveModeIndex(m_cur_frame, n, anim);

            m_remain_time = time - n*frame_time;
            return m_cur_frame;
        }
        uint curFrame() const { return m_cur_frame; };
    private:
        bool m_mirror_b {false};
        uint m_cur_frame {0};
        double m_remain_time {0};
    };
public:
    std::vector<Bone> bones;
    std::vector<Animation> anims;

    Span<Eigen::Affine3f> genFrame(std::vector<AnimationLayer>&, double time);
    void prepared();
private:
    struct AnimationLayer_in {
        const AnimationLayer* layer;
        float blend;
        Animation* anim {nullptr};
    };
    std::vector<Eigen::Affine3f> m_final_affines;
    std::vector<AnimationLayer_in> m_layers_in;
};
}
