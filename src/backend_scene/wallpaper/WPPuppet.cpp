#include "WPPuppet.hpp"

using namespace wallpaper;
using namespace Eigen;

void WPPuppet::prepared() {
    std::vector<Affine3f> combined_tran(bones.size());
    for(uint i=0;i<bones.size();i++) {
        auto& b = bones[i];
        combined_tran[i] = (b.noParent() ? Affine3f::Identity() : combined_tran[b.parent])
            * b.transform;
        
        b.offset_trans = combined_tran[i].inverse();
        b.world_axis_x = (b.offset_trans.linear() * Vector3f::UnitX()).normalized();
        b.world_axis_y = (b.offset_trans.linear() * Vector3f::UnitY()).normalized();
        b.world_axis_z = (b.offset_trans.linear() * Vector3f::UnitZ()).normalized();
    }
}


Span<Eigen::Affine3f> WPPuppet::genFrame(std::vector<AnimationLayer>& layers, double time) {
    m_layers_in.resize(layers.size());
    float blend = 1.0f;
    std::transform(layers.rbegin(), layers.rend(), m_layers_in.rbegin(), [&blend, time, this](auto& layer) {
        float cur_blend {0.0f};
        auto it = std::find_if(anims.begin(), anims.end(), [&layer](auto& a) { return layer.id == a.id; });

        if(layer.visible) {
            cur_blend = blend * layer.blend;
            blend *= 1.0f - layer.blend;
            blend = blend < 0.0f ? 0.0f : blend;

            if(it != anims.end()) layer.nextFrame(time * layer.rate, *it);
        }

        return AnimationLayer_in {
            .layer = &layer,
            .blend = cur_blend,
            .anim = it == anims.end() || !layer.visible ? nullptr : &(*it)
        };
    });


    m_final_affines.resize(bones.size());

    for(uint i=0;i<m_final_affines.size();i++) {
        const auto& bone = bones[i];
        auto& affine = m_final_affines[i];
        affine = Affine3f::Identity();
        assert(bone.parent < i || bone.noParent());
        const Affine3f parent = bone.noParent() ? Affine3f::Identity() : m_final_affines[bone.parent];

        Vector3f trans {Vector3f::Zero()};
        Vector3f scale {Vector3f::Zero()};
        auto linner = affine.linear();

        for(auto& layer:m_layers_in) {
            if(layer.anim == nullptr || !layer.layer->visible) continue;
            assert(i<layer.anim->bframes_array.size());
            if(i>=layer.anim->bframes_array.size()) continue;
            auto iframe = layer.layer->curFrame();
            auto& frame = layer.anim->bframes_array[i].frames[iframe];

            trans += layer.blend * frame.position;// - bone.position);
            const std::array<Vector3f, 3> axis {Vector3f::UnitX(),Vector3f::UnitY(),Vector3f::UnitZ()};
            //const std::array<Vector3f, 3> axis {bone.world_axis_x, bone.world_axis_y, bone.world_axis_z};
            linner *= AngleAxis<float>(layer.blend * frame.angle[2], axis[2]).matrix();
            linner *= AngleAxis<float>(layer.blend * frame.angle[1], axis[1]).matrix();
            linner *= AngleAxis<float>(layer.blend * frame.angle[0], axis[0]).matrix();
            for(uint i=0;i<3;i++) {
                scale[i] += layer.blend * frame.scale[i];
            }
        }
        affine.pretranslate(trans);
        affine.scale(scale);
        affine = parent * affine;
    }

    for(uint i=0;i<m_final_affines.size();i++) {
        m_final_affines[i] *= bones[i].offset_trans.matrix();
    }
    return m_final_affines;
}