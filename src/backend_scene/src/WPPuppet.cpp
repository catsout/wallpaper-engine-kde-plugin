#include "WPPuppet.hpp"
#include <cmath>
#include "Utils/Logging.h"

using namespace wallpaper;
using namespace Eigen;

static Quaterniond ToQuaternion(Vector3f euler) {
    const std::array<Vector3d, 3> axis { Vector3d::UnitX(), Vector3d::UnitY(), Vector3d::UnitZ() };
    return AngleAxis<double>(euler.z(), axis[2]) * AngleAxis<double>(euler.y(), axis[1]) *
           AngleAxis<double>(euler.x(), axis[0]);
};

void WPPuppet::prepared() {
    std::vector<Affine3f> combined_tran(bones.size());
    for (uint i = 0; i < bones.size(); i++) {
        auto& b = bones[i];
        combined_tran[i] =
            (b.noParent() ? Affine3f::Identity() : combined_tran[b.parent]) * b.transform;

        b.offset_trans = combined_tran[i].inverse();
        /*
        b.world_axis_x = (b.offset_trans.linear() *
        Vector3f::UnitX()).normalized(); b.world_axis_y =
        (b.offset_trans.linear() * Vector3f::UnitY()).normalized();
        b.world_axis_z = (b.offset_trans.linear() *
        Vector3f::UnitZ()).normalized();
        */
    }
    for (auto& anim : anims) {
        anim.frame_time = 1.0f / anim.fps;
        anim.max_time   = anim.length / anim.fps;
        for (auto& b : anim.bframes_array) {
            for (auto& f : b.frames) {
                f.quaternion = ToQuaternion(f.angle);
            }
        }
    }

    m_final_affines.resize(bones.size());
}

std::span<const Eigen::Affine3f> WPPuppet::genFrame(WPPuppetLayer& puppet_layer,
                                                    double         time) noexcept {
    double global_blend = puppet_layer.m_global_blend;

    puppet_layer.updateInterpolation(time);

    for (uint i = 0; i < m_final_affines.size(); i++) {
        const auto& bone   = bones[i];
        auto&       affine = m_final_affines[i];

        affine = Affine3f::Identity();
        assert(bone.parent < i || bone.noParent());
        const Affine3f parent =
            bone.noParent() ? Affine3f::Identity() : m_final_affines[bone.parent];

        Vector3f    trans { bone.transform.translation() * global_blend };
        Vector3f    scale { Vector3f::Ones() * global_blend };
        Quaterniond quat { Quaterniond::Identity() };

        // double cur_blend { 0.0f };

        for (auto& layer : puppet_layer.m_layers) {
            auto& alayer = layer.anim_layer;
            if (layer.anim == nullptr || ! alayer.visible) continue;
            assert(i < layer.anim->bframes_array.size());
            if (i >= layer.anim->bframes_array.size()) continue;

            auto&  info    = layer.interp_info;
            auto&  frame_a = layer.anim->bframes_array[i].frames[(usize)info.frame_a];
            auto&  frame_b = layer.anim->bframes_array[i].frames[(usize)info.frame_b];
            double one_t   = 1.0f - info.t;

            quat = frame_a.quaternion.slerp(info.t, frame_b.quaternion)
                       .slerp(1.0f - layer.blend, quat);
            trans += layer.blend * (frame_a.position * one_t + frame_b.position * info.t);
            scale += layer.blend * (frame_a.scale * one_t + frame_b.scale * info.t);
        }
        affine.pretranslate(trans);
        affine.rotate(quat.cast<float>());
        affine.scale(scale);
        affine = parent * affine;
    }

    for (uint i = 0; i < m_final_affines.size(); i++) {
        m_final_affines[i] *= bones[i].offset_trans.matrix();
    }
    return m_final_affines;
}

static constexpr void genInterpolationInfo(WPPuppet::Animation::InterpolationInfo& info,
                                           double& cur, u32 length, double frame_time,
                                           double max_time) {
    cur          = std::fmod(cur, max_time);
    double _rate = cur / frame_time;

    info.frame_a = ((uint)_rate) % length;
    info.frame_b = (info.frame_a + 1) % length;
    info.t       = _rate - (double)info.frame_a;
}

WPPuppet::Animation::InterpolationInfo
WPPuppet::Animation::getInterpolationInfo(double* cur_time) const {
    InterpolationInfo _info;
    auto&             _cur_time = *cur_time;

    if (mode == PlayMode::Loop || mode == PlayMode::Single) {
        genInterpolationInfo(_info, _cur_time, (u32)length, frame_time, max_time);
    } else if (mode == PlayMode::Mirror) {
        const auto _get_frame = [this](auto f) {
            return f >= length ? (length - 1) - (f - length) : f;
        };
        genInterpolationInfo(_info, _cur_time, (u32)length * 2, frame_time, max_time * 2.0f);
        _info.frame_a = _get_frame(_info.frame_a);
        _info.frame_b = _get_frame(_info.frame_b);
    }

    return _info;
}

void WPPuppetLayer::prepared(std::span<AnimationLayer> alayers) {
    m_layers.resize(alayers.size());
    double& blend = m_global_blend;
    std::transform(
        alayers.rbegin(), alayers.rend(), m_layers.rbegin(), [&blend, this](const auto& layer) {
            double      cur_blend { 0.0f };
            const auto& anims = m_puppet->anims;

            auto it = std::find_if(anims.begin(), anims.end(), [&layer](auto& a) {
                return layer.id == a.id;
            });
            bool ok = it != anims.end() && layer.visible;

            if (ok) {
                cur_blend = blend * layer.blend;
                blend *= 1.0f - layer.blend;
                blend = blend < 0.0f ? 0.0f : blend;

                // layer.cur_time += time * layer.rate;
                // info = it->getInterpolationInfo(&(layer.cur_time));
            }

            return Layer {
                .anim_layer = layer,
                .blend      = cur_blend,
                .anim       = ok ? std::addressof(*it) : nullptr,
            };
        });
}

std::span<const Eigen::Affine3f> WPPuppetLayer::genFrame(double time) noexcept {
    return m_puppet->genFrame(*this, time);
}

void WPPuppetLayer::updateInterpolation(double time) noexcept {
    for (auto& layer : m_layers) {
        if (layer) {
            layer.anim_layer.cur_time += time * layer.anim_layer.rate;
            layer.interp_info = layer.anim->getInterpolationInfo(&(layer.anim_layer.cur_time));
        }
    }
}

WPPuppetLayer::WPPuppetLayer(std::shared_ptr<WPPuppet> pup): m_puppet(pup) {}
WPPuppetLayer::WPPuppetLayer()  = default;
WPPuppetLayer::~WPPuppetLayer() = default;
