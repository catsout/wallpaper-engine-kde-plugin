#pragma once
#include "Instance.hpp"
#include "Utils/span.hpp"
#include "Utils/MapSet.hpp"
#include <glslang/Public/ShaderLang.h>
#include <glslang/Include/BaseTypes.h>
#include <vector>
#include <memory>
#include <string>

namespace wallpaper
{
namespace vulkan
{

struct ShaderCompUnit {
    EShLanguage stage;
    std::string src;
};

struct ShaderSpv {
    EShLanguage stage;
    std::vector<unsigned int> spirv;
};

struct ShaderReflected {
    struct BlockedUniform {
        int block_index;
        int offset;
        glslang::TBasicType type;
        size_t num {1}; // for array,vector,matrix
    };
    struct Block {
        int index;
        int size;
        std::string name;
        Map<std::string, BlockedUniform> member_map;
    };
    std::vector<Block> blocks;
    Map<std::string, vk::DescriptorSetLayoutBinding> binding_map;;

    Map<std::string, uint> input_location_map;
};

using Uni_ShaderSpv = std::unique_ptr<ShaderSpv>;

struct ShaderCompOpt {
    glslang::EShTargetClientVersion client_ver;

    bool hlsl {false};
    bool auto_map_locations {false};
    bool auto_map_bindings {false};
    // suppress GLSL warnings, except as required by "#extension : warn"
    bool suppress_warnings_glsl {false};
    // relaxed GLSL semantic error-checking mode
    bool relaxed_errors_glsl {false};
    //  allowing the use of default uniforms, atomic_uints, and gl_VertexID and gl_InstanceID keywords
    bool relaxed_rules_vulkan {false};
};

bool CompileAndLinkShaderUnits(Span<ShaderCompUnit> compUnits, const ShaderCompOpt& opt, std::vector<Uni_ShaderSpv>& spvs, ShaderReflected& reflectd);

}
}