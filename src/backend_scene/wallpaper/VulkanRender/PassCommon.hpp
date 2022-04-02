#pragma once
#include "Vulkan/Instance.hpp"
#include "Type.hpp"
#include "Vulkan/TextureCache.hpp"
#include "Scene/SceneRenderTarget.h"

namespace wallpaper
{
namespace vulkan
{
inline void SetBlend(BlendMode bm, vk::PipelineColorBlendAttachmentState& state) {
    state.setBlendEnable(true);
    state.setColorBlendOp(vk::BlendOp::eAdd).setAlphaBlendOp(vk::BlendOp::eAdd);
    switch (bm) {
    case BlendMode::Disable: state.setBlendEnable(false); break;
    case BlendMode::Normal:
        state.setSrcColorBlendFactor(vk::BlendFactor::eOne)
            .setDstColorBlendFactor(vk::BlendFactor::eZero)
            .setSrcAlphaBlendFactor(vk::BlendFactor::eOne)
            .setDstAlphaBlendFactor(vk::BlendFactor::eZero);
        break;
    case BlendMode::Translucent:
        state.setSrcColorBlendFactor(vk::BlendFactor::eSrcAlpha)
            .setDstColorBlendFactor(vk::BlendFactor::eOneMinusSrcAlpha)
            .setSrcAlphaBlendFactor(vk::BlendFactor::eSrcAlpha)
            .setDstAlphaBlendFactor(vk::BlendFactor::eOneMinusSrcAlpha);
        break;
    case BlendMode::Additive:
        state.setSrcColorBlendFactor(vk::BlendFactor::eSrcAlpha)
            .setDstColorBlendFactor(vk::BlendFactor::eOne)
            .setSrcAlphaBlendFactor(vk::BlendFactor::eSrcAlpha)
            .setDstAlphaBlendFactor(vk::BlendFactor::eOne);
        break;
    }
}
inline void SetAttachmentLoadOp(BlendMode bm, vk::AttachmentLoadOp& load_op) {
    switch (bm) {
    case BlendMode::Disable:
    case BlendMode::Normal: load_op = vk::AttachmentLoadOp::eDontCare; break;
    case BlendMode::Additive:
    case BlendMode::Translucent: load_op = vk::AttachmentLoadOp::eLoad; break;
    }
}

inline TextureKey ToTexKey(wallpaper::SceneRenderTarget rt) {
    return TextureKey { .width        = rt.width,
                        .height       = rt.height,
                        .usage        = {},
                        .format       = wallpaper::TextureFormat::RGBA8,
                        .sample       = rt.sample,
                        .mipmap_level = rt.mipmap_level };
}
} // namespace vulkan
} // namespace wallpaper