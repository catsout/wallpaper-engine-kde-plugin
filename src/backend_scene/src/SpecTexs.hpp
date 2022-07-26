#pragma once
#include <string_view>
#include <cstdint>
#include "Core/Literals.hpp"
#include "Utils/StringHelper.hpp"
#include "Utils/String.h"

namespace wallpaper
{

#define BASE_GLTEX_NAMES(ext)                                                                      \
    "g_Texture0" #ext, "g_Texture1" #ext, "g_Texture2" #ext, "g_Texture3" #ext, "g_Texture4" #ext, \
        "g_Texture5" #ext, "g_Texture6" #ext, "g_Texture7" #ext, "g_Texture8" #ext,                \
        "g_Texture9" #ext, "g_Texture10" #ext, "g_Texture11" #ext, "g_Texture12" #ext

constexpr std::array WE_GLTEX_NAMES { BASE_GLTEX_NAMES() };
constexpr std::array WE_GLTEX_RESOLUTION_NAMES { BASE_GLTEX_NAMES(Resolution) };
constexpr std::array WE_GLTEX_ROTATION_NAMES { BASE_GLTEX_NAMES(Rotation) };
constexpr std::array WE_GLTEX_TRANSLATION_NAMES { BASE_GLTEX_NAMES(Translation) };
constexpr std::array WE_GLTEX_MIPMAPINFO_NAMES { BASE_GLTEX_NAMES(MipMapInfo) };
#undef BASE_GLTEX_NAMES

constexpr std::string_view WE_SPEC_PREFIX { "_rt_" };
constexpr std::string_view WE_IMAGE_LAYER_COMPOSITE_PREFIX { "_rt_imageLayerComposite_" };
constexpr std::string_view WE_HALF_COMPO_BUFFER_PREFIX { "_rt_HalfCompoBuffer" };
constexpr std::string_view WE_QUARTER_COMPO_BUFFER_PREFIX { "_rt_QuarterCompoBuffer" };
constexpr std::string_view WE_FULL_COMPO_BUFFER_PREFIX { "_rt_FullCompoBuffer" };
constexpr std::string_view WE_MIP_MAPPED_FRAME_BUFFER { "_rt_MipMappedFrameBuffer" };

constexpr std::string_view WE_EFFECT_PPONG_PREFIX { "_rt_effect_pingpong_" };
constexpr std::string_view WE_EFFECT_PPONG_PREFIX_A { "_rt_effect_pingpong_a_" };
constexpr std::string_view WE_EFFECT_PPONG_PREFIX_B { "_rt_effect_pingpong_b_" };

constexpr std::string_view WE_IN_POSITION { "a_Position" };
constexpr std::string_view WE_IN_TEXCOORD { "a_TexCoord" };
constexpr std::string_view WE_IN_BLENDINDICES { "a_BlendIndices" };
constexpr std::string_view WE_IN_BLENDWEIGHTS { "a_BlendWeights" };

// particle

constexpr std::string_view WE_IN_POSITIONVEC4 { "a_PositionVec4" };
constexpr std::string_view WE_IN_COLOR { "a_Color" };
constexpr std::string_view WE_IN_TEXCOORDVEC4 { "a_TexCoordVec4" };
constexpr std::string_view WE_IN_TEXCOORDVEC4C1 { "a_TexCoordVec4C1" };
constexpr std::string_view WE_IN_TEXCOORDVEC4C2 { "a_TexCoordVec4C2" };
constexpr std::string_view WE_IN_TEXCOORDVEC4C3 { "a_TexCoordVec4C3" };
constexpr std::string_view WE_IN_TEXCOORDVEC3C2 { "a_TexCoordVec3C2" };
constexpr std::string_view WE_IN_TEXCOORDC2 { "a_TexCoordC2" };
constexpr std::string_view WE_IN_TEXCOORDC3 { "a_TexCoordC3" };
constexpr std::string_view WE_IN_TEXCOORDC4 { "a_TexCoordC4" };
constexpr std::string_view WE_CB_THICK_FORMAT { "THICKFORMAT" };
constexpr std::string_view WE_PRENDER_ROPE { "PRENDER_ROPE" };

constexpr std::string_view G_M { "g_ModelMatrix" };
constexpr std::string_view G_VP { "g_ViewProjectionMatrix" };
constexpr std::string_view G_MVP { "g_ModelViewProjectionMatrix" };
constexpr std::string_view G_AM { "g_AltModelMatrix" };
constexpr std::string_view G_MI { "g_ModelMatrixInverse" };
constexpr std::string_view G_MVPI { "g_ModelViewProjectionMatrixInverse" };
constexpr std::string_view G_ETVP { "g_EffectTextureProjectionMatrix" };
constexpr std::string_view G_ETVPI { "g_EffectTextureProjectionMatrixInverse" };
constexpr std::string_view G_LP { "g_LightsPosition" };
constexpr std::string_view G_LCP { "g_LightsColorPremultiplied" };

constexpr std::string_view G_TIME { "g_Time" };
constexpr std::string_view G_DAYTIME { "g_DayTime" };
constexpr std::string_view G_POINTERPOSITION { "g_PointerPosition" };
constexpr std::string_view G_TEXELSIZE { "g_TexelSize" };
constexpr std::string_view G_TEXELSIZEHALF { "g_TexelSizeHalf" };
constexpr std::string_view G_BONES { "g_Bones" };
constexpr std::string_view G_SCREEN { "g_Screen" };
constexpr std::string_view G_PARALLAXPOSITION { "g_ParallaxPosition" };

constexpr std::string_view SpecTex_Default { "_rt_default" };
constexpr std::string_view SpecTex_Link { "_rt_link_" };

inline bool IsSpecTex(const std::string_view name) { return sstart_with(name, WE_SPEC_PREFIX); }
inline bool IsSpecLinkTex(const std::string_view name) { return sstart_with(name, SpecTex_Link); }
inline uint32_t ParseLinkTex(const std::string_view name) {
    std::string sid { name };
    sid = sid.substr(9);
    uint32_t result { 0 };
    STRTONUM(sid, result);
    return result;
}
inline std::string GenLinkTex(idx id) { return std::string(SpecTex_Link) + std::to_string(id); }

} // namespace wallpaper
