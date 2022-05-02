#pragma once
#include "Instance.hpp"
#include "Utils/span.hpp"
#include "Utils/MapSet.hpp"
#include "Spv.hpp"
#include <glslang/Public/ShaderLang.h>
#include <glslang/Include/BaseTypes.h>
#include <vector>
#include <memory>
#include <string>

namespace wallpaper
{
namespace vulkan
{

extern const TBuiltInResource DefaultTBuiltInResource;

struct ShaderCompUnit {
    EShLanguage stage;
    std::string src;
};
vk::Format ToVkType(glslang::TBasicType, size_t);
vk::ShaderStageFlags ToVkType(EShLanguageMask);
vk::ShaderStageFlagBits ToVkType_Stage(EShLanguage);
size_t Sizeof(glslang::TBasicType);

struct ShaderReflected {
    struct BlockedUniform {
        int block_index;
        int offset;
        glslang::TBasicType type {};
        size_t num {1}; // for array,vector,matrix
    };
    struct Block {
        int index;
        int size;
        std::string name;
        Map<std::string, BlockedUniform> member_map;
    };
    std::vector<Block> blocks;
    Map<std::string, vk::DescriptorSetLayoutBinding> binding_map;

    struct Input {
        uint location;
        vk::Format format;
    };
    Map<std::string, Input> input_location_map;
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

bool CompileAndLinkShaderUnits(Span<const ShaderCompUnit> compUnits, const ShaderCompOpt& opt, std::vector<Uni_ShaderSpv>& spvs, ShaderReflected* reflectd);

}
}