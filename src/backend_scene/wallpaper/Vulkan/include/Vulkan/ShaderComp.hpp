#pragma once
#include "Utils/span.hpp"
#include "Spv.hpp"
#include <glslang/Public/ShaderLang.h>


namespace wallpaper
{
namespace vulkan
{

extern const TBuiltInResource DefaultTBuiltInResource;

struct ShaderCompUnit {
    EShLanguage stage;
    std::string src;
};

struct ShaderCompOpt {
    glslang::EShTargetClientVersion client_ver;

    bool hlsl {false};
    bool auto_map_locations {false};
    bool auto_map_bindings {false};
    // suppress GLSL warnings, except as required by "#extension : warn"
    bool suppress_warnings_glsl {false};
    // relaxed GLSL semantic error-checking mode
    bool relaxed_errors_glsl {false};
    // allowing the use of default uniforms, atomic_uints, and gl_VertexID and gl_InstanceID keywords
    bool relaxed_rules_vulkan {false};
    // for global unifom block
    uint global_uniform_binding {0};

    // relfect:
    bool reflect_all_io_var {true};
    bool reflect_all_block_var {false};
};

bool CompileAndLinkShaderUnits(Span<const ShaderCompUnit> compUnits, const ShaderCompOpt& opt, std::vector<Uni_ShaderSpv>& spvs);
}
}
