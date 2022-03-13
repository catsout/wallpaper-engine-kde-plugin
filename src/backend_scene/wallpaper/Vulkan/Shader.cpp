#include "Shader.hpp"

#include <glslang/MachineIndependent/iomapper.h>
#include <glslang/SPIRV/GlslangToSpv.h>
#include "Utils/Logging.h"
#include <cstdlib>
#include <string>
#include "Utils/Sha.hpp"

using namespace wallpaper::vulkan;

#define _VK_FORMAT_1(s, sign, type, x)          vk::Format::e##x ##s ##sign ##type;
#define _VK_FORMAT_2(s, sign, type, x, y)       vk::Format::e##x ##s ##y ##s ##sign ##type;
#define _VK_FORMAT_3(s, sign, type, x, y, z)    vk::Format::e##x ##s ##y ##s ##z ##s ##sign ##type;
#define _VK_FORMAT_4(s, sign, type, x, y, z, w) vk::Format::e##x ##s ##y ##s ##z ##s ##w ##s ##sign ##type;


namespace wallpaper 
{

vk::ShaderStageFlags vulkan::ToVkType(EShLanguageMask mask) {
	vk::ShaderStageFlags flags {};
	if(mask & EShLangVertexMask)
		flags |= vk::ShaderStageFlagBits::eVertex;
	if(mask & EShLangFragmentMask)
		flags |= vk::ShaderStageFlagBits::eFragment;

	return flags;
}
vk::ShaderStageFlagBits vulkan::ToVkType_Stage(EShLanguage lan) {
	switch (lan)
	{
	case EShLangVertex: return vk::ShaderStageFlagBits::eVertex;
	case EShLangFragment: return vk::ShaderStageFlagBits::eFragment;
	}
	return vk::ShaderStageFlagBits::eVertex;
}
}

int ClientInputSemanticsVersion = 100;

const TBuiltInResource DefaultTBuiltInResource = {
	/* .MaxLights = */ 32,
	/* .MaxClipPlanes = */ 6,
	/* .MaxTextureUnits = */ 32,
	/* .MaxTextureCoords = */ 32,
	/* .MaxVertexAttribs = */ 64,
	/* .MaxVertexUniformComponents = */ 4096,
	/* .MaxVaryingFloats = */ 64,
	/* .MaxVertexTextureImageUnits = */ 32,
	/* .MaxCombinedTextureImageUnits = */ 80,
	/* .MaxTextureImageUnits = */ 32,
	/* .MaxFragmentUniformComponents = */ 4096,
	/* .MaxDrawBuffers = */ 32,
	/* .MaxVertexUniformVectors = */ 128,
	/* .MaxVaryingVectors = */ 8,
	/* .MaxFragmentUniformVectors = */ 16,
	/* .MaxVertexOutputVectors = */ 16,
	/* .MaxFragmentInputVectors = */ 15,
	/* .MinProgramTexelOffset = */ -8,
	/* .MaxProgramTexelOffset = */ 7,
	/* .MaxClipDistances = */ 8,
	/* .MaxComputeWorkGroupCountX = */ 65535,
	/* .MaxComputeWorkGroupCountY = */ 65535,
	/* .MaxComputeWorkGroupCountZ = */ 65535,
	/* .MaxComputeWorkGroupSizeX = */ 1024,
	/* .MaxComputeWorkGroupSizeY = */ 1024,
	/* .MaxComputeWorkGroupSizeZ = */ 64,
	/* .MaxComputeUniformComponents = */ 1024,
	/* .MaxComputeTextureImageUnits = */ 16,
	/* .MaxComputeImageUniforms = */ 8,
	/* .MaxComputeAtomicCounters = */ 8,
	/* .MaxComputeAtomicCounterBuffers = */ 1,
	/* .MaxVaryingComponents = */ 60,
	/* .MaxVertexOutputComponents = */ 64,
	/* .MaxGeometryInputComponents = */ 64,
	/* .MaxGeometryOutputComponents = */ 128,
	/* .MaxFragmentInputComponents = */ 128,
	/* .MaxImageUnits = */ 8,
	/* .MaxCombinedImageUnitsAndFragmentOutputs = */ 8,
	/* .MaxCombinedShaderOutputResources = */ 8,
	/* .MaxImageSamples = */ 0,
	/* .MaxVertexImageUniforms = */ 0,
	/* .MaxTessControlImageUniforms = */ 0,
	/* .MaxTessEvaluationImageUniforms = */ 0,
	/* .MaxGeometryImageUniforms = */ 0,
	/* .MaxFragmentImageUniforms = */ 8,
	/* .MaxCombinedImageUniforms = */ 8,
	/* .MaxGeometryTextureImageUnits = */ 16,
	/* .MaxGeometryOutputVertices = */ 256,
	/* .MaxGeometryTotalOutputComponents = */ 1024,
	/* .MaxGeometryUniformComponents = */ 1024,
	/* .MaxGeometryVaryingComponents = */ 64,
	/* .MaxTessControlInputComponents = */ 128,
	/* .MaxTessControlOutputComponents = */ 128,
	/* .MaxTessControlTextureImageUnits = */ 16,
	/* .MaxTessControlUniformComponents = */ 1024,
	/* .MaxTessControlTotalOutputComponents = */ 4096,
	/* .MaxTessEvaluationInputComponents = */ 128,
	/* .MaxTessEvaluationOutputComponents = */ 128,
	/* .MaxTessEvaluationTextureImageUnits = */ 16,
	/* .MaxTessEvaluationUniformComponents = */ 1024,
	/* .MaxTessPatchComponents = */ 120,
	/* .MaxPatchVertices = */ 32,
	/* .MaxTessGenLevel = */ 64,
	/* .MaxViewports = */ 16,
	/* .MaxVertexAtomicCounters = */ 0,
	/* .MaxTessControlAtomicCounters = */ 0,
	/* .MaxTessEvaluationAtomicCounters = */ 0,
	/* .MaxGeometryAtomicCounters = */ 0,
	/* .MaxFragmentAtomicCounters = */ 8,
	/* .MaxCombinedAtomicCounters = */ 8,
	/* .MaxAtomicCounterBindings = */ 1,
	/* .MaxVertexAtomicCounterBuffers = */ 0,
	/* .MaxTessControlAtomicCounterBuffers = */ 0,
	/* .MaxTessEvaluationAtomicCounterBuffers = */ 0,
	/* .MaxGeometryAtomicCounterBuffers = */ 0,
	/* .MaxFragmentAtomicCounterBuffers = */ 1,
	/* .MaxCombinedAtomicCounterBuffers = */ 1,
	/* .MaxAtomicCounterBufferSize = */ 16384,
	/* .MaxTransformFeedbackBuffers = */ 4,
	/* .MaxTransformFeedbackInterleavedComponents = */ 64,
	/* .MaxCullDistances = */ 8,
	/* .MaxCombinedClipAndCullDistances = */ 8,
	/* .MaxSamples = */ 4,
	/* .maxMeshOutputVerticesNV = */ 256,
	/* .maxMeshOutputPrimitivesNV = */ 512,
	/* .maxMeshWorkGroupSizeX_NV = */ 32,
	/* .maxMeshWorkGroupSizeY_NV = */ 1,
	/* .maxMeshWorkGroupSizeZ_NV = */ 1,
	/* .maxTaskWorkGroupSizeX_NV = */ 32,
	/* .maxTaskWorkGroupSizeY_NV = */ 1,
	/* .maxTaskWorkGroupSizeZ_NV = */ 1,
	/* .maxMeshViewCountNV = */ 4,
	/* .maxDualSourceDrawBuffersEXT = */ 1,

	/* .limits = */ {
		/* .nonInductiveForLoops = */ 1,
		/* .whileLoops = */ 1,
		/* .doWhileLoops = */ 1,
		/* .generalUniformIndexing = */ 1,
		/* .generalAttributeMatrixVectorIndexing = */ 1,
		/* .generalVaryingIndexing = */ 1,
		/* .generalSamplerIndexing = */ 1,
		/* .generalVariableIndexing = */ 1,
		/* .generalConstantMatrixVectorIndexing = */ 1,
	}};


static glslang::EShClient getClient(glslang::EShTargetClientVersion ClientVersion) {
    switch (ClientVersion)
    {
    case glslang::EShTargetVulkan_1_0:
    case glslang::EShTargetVulkan_1_1:
    case glslang::EShTargetVulkan_1_2:
        return glslang::EShClientVulkan;
    case glslang::EShTargetOpenGL_450:
        return glslang::EShClientOpenGL;
    default:
        return glslang::EShClientVulkan;
    }
}
static glslang::EShTargetLanguageVersion getTargetVersion(glslang::EShTargetClientVersion ClientVersion) { 
    glslang::EShTargetLanguageVersion TargetVersion;
    switch (ClientVersion) {
    case glslang::EShTargetVulkan_1_0:
        TargetVersion = glslang::EShTargetSpv_1_0;
        break;
    case glslang::EShTargetVulkan_1_1:
        TargetVersion = glslang::EShTargetSpv_1_3;
        break;
    case glslang::EShTargetVulkan_1_2:
        TargetVersion = glslang::EShTargetSpv_1_5;
        break;
    case glslang::EShTargetVulkan_1_3:
        TargetVersion = glslang::EShTargetSpv_1_6;
        break;
    case glslang::EShTargetOpenGL_450:
        TargetVersion = glslang::EShTargetSpv_1_0;
        break;
    default:
        break;
    }
    return TargetVersion;
}

static bool parse(const ShaderCompUnit& unit, const ShaderCompOpt& opt, EShMessages emsg, glslang::TShader& shader) {
    auto* data = unit.src.c_str();
    auto client = getClient(opt.client_ver);
    shader.setStrings(&data, 1);
    shader.setEnvInput(opt.hlsl ? glslang::EShSourceHlsl 
                                : glslang::EShSourceGlsl, EShLanguage::EShLangVertex,
                       client, ClientInputSemanticsVersion);
    shader.setEnvClient(client, opt.client_ver);
    shader.setEnvTarget(glslang::EShTargetLanguage::EShTargetSpv, getTargetVersion(opt.client_ver));
    if(opt.auto_map_locations)
        shader.setAutoMapLocations(true);
    if(opt.auto_map_bindings)
        shader.setAutoMapBindings(true);
    if(opt.relaxed_rules_vulkan) {
		shader.setGlobalUniformBinding(opt.global_uniform_binding);
        shader.setEnvInputVulkanRulesRelaxed();
	}
    const int default_ver = 100; // or 110
    TBuiltInResource resource = DefaultTBuiltInResource;
    if (!shader.parse(&resource, default_ver, false, emsg)) {
		std::string tmp_name = logToTmpfileWithSha1(unit.src, "%s", unit.src.c_str());
		LOG_ERROR("shader source is at %s", tmp_name.c_str());
		LOG_ERROR("glslang(parse): %s", shader.getInfoLog());
        return false;
    }
    return true;
}

static void SetMessageOptions(const ShaderCompOpt& opt, EShMessages& emsg) {
    emsg = (EShMessages)(EShMsgDefault | EShMsgSpvRules);
    if(opt.relaxed_errors_glsl)
        emsg = (EShMessages)(emsg | EShMsgRelaxedErrors);
    if(opt.suppress_warnings_glsl)
        emsg = (EShMessages)(emsg | EShMsgSuppressWarnings);
    if(getClient(opt.client_ver) == glslang::EShClientVulkan)
        emsg = (EShMessages)(emsg | EShMsgVulkanRules);
}


static size_t GetTypeNum(const glslang::TType* type) {
	size_t num {1};
	if(type->isArray())  num *= type->getCumulativeArraySize();
	if(type->isVector()) num *= type->getVectorSize();
	if(type->isMatrix()) num *= type->getMatrixCols() * type->getMatrixRows();
	return num;
}

static bool GetReflectedInfo(glslang::TProgram& pro, ShaderReflected& ref) {
	if(!pro.buildReflection()) return false;
	int numBlocks = pro.getNumUniformBlocks();
	int numUnfis = pro.getNumUniformVariables();
	for(int i=0;i<numBlocks;i++) {
		auto& block = pro.getUniformBlock(i);
		ref.blocks.push_back(ShaderReflected::Block{
			.index = i,
			.size = block.size,
			.name = block.name,
			.member_map = {}
		});
		vk::DescriptorSetLayoutBinding binding;
		binding.setBinding(block.getBinding())
			.setDescriptorCount(1)
			.setDescriptorType(vk::DescriptorType::eUniformBuffer)
			.setStageFlags(ToVkType(block.stages));
		ref.binding_map[block.name] = binding;
	}
	for(int i=0;i<numUnfis;i++) {
		auto& unif = pro.getUniform(i);
		auto* type = unif.getType();
		auto basic_type = type->getBasicType();
		if(type->isTexture()) {
			vk::DescriptorSetLayoutBinding binding;
			binding.setBinding(unif.getBinding())
				.setDescriptorCount(1)
				.setDescriptorType(vk::DescriptorType::eCombinedImageSampler)
				.setStageFlags(ToVkType(unif.stages));
			ref.binding_map[unif.name] = binding;	
		} else if(!type->isStruct()) {
			// in block
			if(unif.index >= ref.blocks.size()) 
				return false;
			auto& block = ref.blocks[unif.index];
			ShaderReflected::BlockedUniform bunif {};
			{
				bunif.num = GetTypeNum(type);
				bunif.type = type->getBasicType();
				bunif.block_index = unif.index;
				bunif.offset = unif.offset;
			}
			block.member_map[unif.name] = bunif;
		}
	}
	int numInputs = pro.getNumPipeInputs();
	for(int i=0;i<numInputs;i++) {
		auto& input = pro.getPipeInput(i);
		auto* type = input.getType();
		auto& qual = type->getQualifier();

		if(!qual.hasAnyLocation()) return false;
		ShaderReflected::Input rinput;
		rinput.location = qual.layoutLocation;
		rinput.format = ToVkType(type->getBasicType(), GetTypeNum(type));
		ref.input_location_map[input.name] = rinput;
	}

	return true;
};

bool wallpaper::vulkan::CompileAndLinkShaderUnits(Span<ShaderCompUnit> compUnits, const ShaderCompOpt& opt, std::vector<Uni_ShaderSpv>& spvs, ShaderReflected* reflectd) {
    glslang::TProgram program;
    EShMessages emsg;
    SetMessageOptions(opt, emsg);
    std::vector<std::unique_ptr<glslang::TShader>> shaders;
    for(auto& unit:compUnits) {
        shaders.emplace_back(std::make_unique<glslang::TShader>(unit.stage));
        auto& shader = *(shaders.back());
        if(!parse(unit, opt, emsg, shader))
			return false;
        program.addShader(&shader);
    }

    if (!program.link(emsg)) {
        LOG_ERROR("glslang(link): %s\n", program.getInfoLog());   
        return false;
    }

    for(auto& unit:compUnits) { (void)program.getIntermediate(unit.stage); }
	glslang::TIntermediate* firstIm = program.getIntermediate(compUnits[0].stage);
    glslang::TDefaultGlslIoResolver resolver(*firstIm);
    glslang::TGlslIoMapper ioMapper;

	if(!( program.mapIO(&resolver, &ioMapper) && 
		(reflectd == nullptr || GetReflectedInfo(program, *reflectd)) )) {
        LOG_ERROR("glslang(mapIo): %s\n", program.getInfoLog());   
		return false;
	}

    spv::SpvBuildLogger logger;
    glslang::SpvOptions spvOptions;
    spvOptions.validate = true;
    spvOptions.generateDebugInfo = true;

    spvs.clear();
    for(auto& unit:compUnits) {
        Uni_ShaderSpv spv = std::make_unique<ShaderSpv>();
        spv->stage = ToVkType_Stage(unit.stage);
        auto im = program.getIntermediate(unit.stage);
        im->setOriginUpperLeft();
        glslang::GlslangToSpv(*im, spv->spirv, &logger, &spvOptions);
		//glslang::OutputSpvBin(spv->spirv, (std::to_string((int)spv->stage) + ".spv").c_str());
        spvs.emplace_back(std::move(spv)); 
        
        auto messages = logger.getAllMessages();
        if (messages.length() > 0)
            LOG_ERROR("glslang(spv): %s\n", messages.c_str());
    }

    return true;
}

vk::Format wallpaper::vulkan::ToVkType(glslang::TBasicType type, size_t size) {
#define FORMAT_SWITCH(in, s, sign, type)                        \
		switch (in) {                                           \
		case 1: return _VK_FORMAT_1(s, sign, type, R);          \
		case 2: return _VK_FORMAT_2(s, sign, type, R, G);       \
		case 3: return _VK_FORMAT_3(s, sign, type, R, G, B);    \
		case 4: return _VK_FORMAT_4(s, sign, type, R, G, B, A); \
		}                                                       \
		break;

	switch (type)
	{
	case glslang::TBasicType::EbtFloat:
		FORMAT_SWITCH(size, 32, S, float);
	case glslang::TBasicType::EbtInt:
		FORMAT_SWITCH(size, 32, S, int);
	case glslang::TBasicType::EbtUint:
		FORMAT_SWITCH(size, 32, U, int);
	}
	LOG_ERROR("can't covert glslang type \"%s\" of size %d to vulkan format", glslang::TType::getBasicString(type), size);
	assert(false);
	return vk::Format::eUndefined;
#undef FORMAT_SWITCH
}

size_t wallpaper::vulkan::Sizeof(glslang::TBasicType type) {
	switch (type)
	{
	case glslang::TBasicType::EbtFloat:
	case glslang::TBasicType::EbtInt:
		return sizeof(float);
	}
	LOG_ERROR("can't get glslang type \"%s\" size", glslang::TType::getBasicString(type));
	assert(false);
	return 0;
}