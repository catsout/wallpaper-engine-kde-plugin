#include "WPSceneParser.hpp"
#include "WPJson.hpp"

#include "Utils/String.h"
#include "Utils/Logging.h"
#include "Utils/Algorism.h"
#include "Utils/ArrayHelper.hpp"
#include "Utils/Visitors.hpp"
#include "Utils/StringHelper.hpp"
#include "SpecTexs.hpp"

#include "WPShaderParser.hpp"
#include "WPTexImageParser.hpp"
#include "WPParticleParser.hpp"
#include "WPSoundParser.hpp"
#include "WPMdlParser.hpp"

#include "Particle/WPParticleRawGener.h"
#include "Particle/ParticleSystem.h"

#include "WPShaderValueUpdater.hpp"
#include "wpscene/WPImageObject.h"
#include "wpscene/WPParticleObject.h"
#include "wpscene/WPSoundObject.h"
#include "wpscene/WPLightObject.hpp"
#include "wpscene/WPScene.h"

#include "Fs/VFS.h"

#include <iostream>
#include <sstream>
#include <string>
#include <unordered_map>
#include <random>
#include <cmath>
#include <functional>
#include <regex>
#include <variant>
#include <Eigen/Dense>

using namespace wallpaper;
using namespace Eigen;

typedef std::function<float()> RandomFn;

std::string getAddr(void* p) { return std::to_string(reinterpret_cast<intptr_t>(p)); }

struct ParseContext {
    RandomFn               randomFn;
    std::shared_ptr<Scene> scene;
    WPShaderValueUpdater*  shader_updater;
    uint16_t               ortho_w;
    uint16_t               ortho_h;
    fs::VFS*               vfs;

    ShaderValueMap             global_base_uniforms;
    std::shared_ptr<SceneNode> effect_camera_node;
    std::shared_ptr<SceneNode> global_camera_node;
    std::shared_ptr<SceneNode> global_perspective_camera_node;
};

using WPObjectVar = std::variant<wpscene::WPImageObject, wpscene::WPParticleObject,
                                 wpscene::WPSoundObject, wpscene::WPLightObject>;

namespace
{
// mapRate < 1.0
void GenCardMesh(SceneMesh& mesh, const std::array<uint16_t, 2> size,
                 const std::array<float, 2> mapRate = { 1.0f, 1.0f }) {
    float left   = -(size[0] / 2.0f);
    float right  = size[0] / 2.0f;
    float bottom = -(size[1] / 2.0f);
    float top    = size[1] / 2.0f;
    float z      = 0.0f;

    float tw = mapRate[0], th = mapRate[1];

    // clang-format off
	const std::array pos = {
		left, bottom, z,
		left,  top, z,
		right, bottom, z,
		right,  top, z,
	};
	const std::array texCoord = {
		0.0f, th,
		0.0f, 0.0f,
		tw, th,
		tw, 0.0f,
	};
    // clang-format on

    SceneVertexArray vertex(
        {
            { WE_IN_POSITION.data(), VertexType::FLOAT3 },
            { WE_IN_TEXCOORD.data(), VertexType::FLOAT2 },
        },
        4);
    vertex.SetVertex(WE_IN_POSITION, pos);
    vertex.SetVertex(WE_IN_TEXCOORD, texCoord);
    mesh.AddVertexArray(std::move(vertex));
}

void SetParticleMesh(SceneMesh& mesh, const wpscene::Particle& particle, uint32_t count,
                     bool thick_format) {
    std::vector<SceneVertexArray::SceneVertexAttribute> attrs {
        { WE_IN_POSITION.data(), VertexType::FLOAT3 },
        { WE_IN_TEXCOORDVEC4.data(), VertexType::FLOAT4 },
        { WE_IN_COLOR.data(), VertexType::FLOAT4 },
    };
    if (thick_format) {
        attrs.push_back({ WE_IN_TEXCOORDVEC4C1.data(), VertexType::FLOAT4 });
    }
    attrs.push_back({ WE_IN_TEXCOORDC2.data(), VertexType::FLOAT2 });
    mesh.AddVertexArray(SceneVertexArray(attrs, count * 4));
    mesh.AddIndexArray(SceneIndexArray(count));
    mesh.GetVertexArray(0).SetOption(WE_CB_THICK_FORMAT, thick_format);
}

void SetRopeParticleMesh(SceneMesh& mesh, const wpscene::Particle& particle, uint32_t count,
                         bool thick_format) {
    std::vector<SceneVertexArray::SceneVertexAttribute> attrs {
        { WE_IN_POSITIONVEC4.data(), VertexType::FLOAT4 },
        { WE_IN_TEXCOORDVEC4.data(), VertexType::FLOAT4 },
        { WE_IN_TEXCOORDVEC4C1.data(), VertexType::FLOAT4 },
    };
    if (thick_format) {
        attrs.push_back({ WE_IN_TEXCOORDVEC4C2.data(), VertexType::FLOAT4 });
        attrs.push_back({ WE_IN_TEXCOORDVEC4C3.data(), VertexType::FLOAT4 });
        attrs.push_back({ WE_IN_TEXCOORDC4.data(), VertexType::FLOAT4 });
    } else {
        attrs.push_back({ WE_IN_TEXCOORDVEC3C2.data(), VertexType::FLOAT4 });
        attrs.push_back({ WE_IN_TEXCOORDC3.data(), VertexType::FLOAT4 });
    }
    attrs.push_back({ WE_IN_COLOR.data(), VertexType::FLOAT4 });
    mesh.AddVertexArray(SceneVertexArray(attrs, count * 4));
    mesh.AddIndexArray(SceneIndexArray(count));
    mesh.GetVertexArray(0).SetOption(WE_PRENDER_ROPE, true);
    mesh.GetVertexArray(0).SetOption(WE_CB_THICK_FORMAT, thick_format);
}

ParticleAnimationMode ToAnimMode(const std::string& str) {
    if (str == "randomframe")
        return ParticleAnimationMode::RANDOMONE;
    else if (str == "sequence")
        return ParticleAnimationMode::SEQUENCE;
    else {
        return ParticleAnimationMode::SEQUENCE;
    }
}

void LoadInitializer(ParticleSubSystem& pSys, const wpscene::Particle& wp,
                     const wpscene::ParticleInstanceoverride& over, RandomFn& randomFn) {
    for (const auto& ini : wp.initializers) {
        pSys.AddInitializer(WPParticleParser::genParticleInitOp(ini, randomFn));
    }
    if (over.enabled) pSys.AddInitializer(WPParticleParser::genOverrideInitOp(over));
}
void LoadOperator(ParticleSubSystem& pSys, const wpscene::Particle& wp,
                  const wpscene::ParticleInstanceoverride& over, RandomFn& randomFn) {
    for (const auto& op : wp.operators) {
        pSys.AddOperator(WPParticleParser::genParticleOperatorOp(op, randomFn, over));
    }
}
void LoadEmitter(ParticleSubSystem& pSys, const wpscene::Particle& wp, float count,
                 RandomFn& randomFn, bool render_rope) {
    bool sort = render_rope;
    for (const auto& em : wp.emitters) {
        auto newEm = em;
        newEm.rate *= count;
        // newEm.origin[2] -= perspectiveZ;
        pSys.AddEmitter(WPParticleParser::genParticleEmittOp(newEm, randomFn, sort));
    }
}

BlendMode ParseBlendMode(std::string_view str) {
    BlendMode bm;
    if (str == "translucent") {
        bm = BlendMode::Translucent;
    } else if (str == "additive") {
        bm = BlendMode::Additive;
    } else if (str == "normal") {
        bm = BlendMode::Normal;
    } else if (str == "disabled") {
        // seems disabled is normal
        bm = BlendMode::Normal;
    } else {
        LOG_ERROR("unknown blending: %s", str.data());
    }
    return bm;
}

void ParseSpecTexName(std::string& name, const wpscene::WPMaterial& wpmat,
                      const WPShaderInfo& sinfo) {
    if (IsSpecTex(name)) {
        if (name == "_rt_FullFrameBuffer") {
            name = SpecTex_Default;
            if (wpmat.shader == "genericimage2" && ! exists(sinfo.combos, "BLENDMODE")) name = "";
            /*
            if(wpmat.shader == "genericparticle") {
                name = "_rt_ParticleRefract";
            }
            */
        } else if (sstart_with(name, WE_IMAGE_LAYER_COMPOSITE_PREFIX)) {
            LOG_INFO("link tex \"%s\"", name.c_str());
            int         wpid { -1 };
            std::regex  reImgId { R"(_rt_imageLayerComposite_([0-9]+))" };
            std::smatch match;
            if (std::regex_search(name, match, reImgId)) {
                STRTONUM(std::string(match[1]), wpid);
            }
            name = GenLinkTex(wpid);
        } else if (sstart_with(name, WE_MIP_MAPPED_FRAME_BUFFER)) {
        } else if (sstart_with(name, WE_EFFECT_PPONG_PREFIX)) {
        } else if (sstart_with(name, WE_HALF_COMPO_BUFFER_PREFIX)) {
        } else if (sstart_with(name, WE_QUARTER_COMPO_BUFFER_PREFIX)) {
        } else if (sstart_with(name, WE_FULL_COMPO_BUFFER_PREFIX)) {
        } else {
            LOG_ERROR("unknown tex \"%s\"", name.c_str());
        }
    }
}

bool LoadMaterial(fs::VFS& vfs, const wpscene::WPMaterial& wpmat, Scene* pScene, SceneNode* pNode,
                  SceneMaterial* pMaterial, WPShaderValueData* pSvData,
                  WPShaderInfo* pWPShaderInfo = nullptr) {
    auto& svData   = *pSvData;
    auto& material = *pMaterial;

    std::unique_ptr<WPShaderInfo> upWPShaderInfo(nullptr);
    if (pWPShaderInfo == nullptr) {
        upWPShaderInfo = std::make_unique<WPShaderInfo>();
        pWPShaderInfo  = upWPShaderInfo.get();
    }

    SceneMaterialCustomShader materialShader;

    auto& shader = materialShader.shader;
    shader       = std::make_shared<SceneShader>();
    shader->name = wpmat.shader;

    std::string shaderPath("/assets/shaders/" + wpmat.shader);

    std::array sd_units { WPShaderUnit { .stage = ShaderType::VERTEX,
                                         .src   = fs::GetFileContent(vfs, shaderPath + ".vert") },
                          WPShaderUnit { .stage = ShaderType::FRAGMENT,
                                         .src   = fs::GetFileContent(vfs, shaderPath + ".frag") } };

    std::vector<WPShaderTexInfo>                 texinfos;
    std::unordered_map<std::string, ImageHeader> texHeaders;
    for (const auto& el : wpmat.textures) {
        if (el.empty()) {
            texinfos.push_back({ false });
        } else if (! IsSpecTex(el)) {
            const auto& texh = pScene->imageParser->ParseHeader(el);
            texHeaders[el]   = texh;
            if (texh.extraHeader.count("compo1") == 0) {
                texinfos.push_back({ false });
                continue;
            }
            texinfos.push_back({ true,
                                 {
                                     (bool)texh.extraHeader.at("compo1").val,
                                     (bool)texh.extraHeader.at("compo2").val,
                                     (bool)texh.extraHeader.at("compo3").val,
                                 } });
        } else
            texinfos.push_back({ true });
    }

    for (auto& unit : sd_units) {
        unit.src = WPShaderParser::PreShaderSrc(vfs, unit.src, pWPShaderInfo, texinfos);
    }

    shader->default_uniforms = pWPShaderInfo->svs;

    for (const auto& el : wpmat.combos) {
        pWPShaderInfo->combos[el.first] = std::to_string(el.second);
    }

    auto textures = wpmat.textures;
    if (pWPShaderInfo->defTexs.size() > 0) {
        for (auto& t : pWPShaderInfo->defTexs) {
            if (textures.size() > t.first) {
                if (! textures.at(t.first).empty()) continue;
            } else {
                textures.resize(t.first + 1);
            }
            textures[t.first] = t.second;
        }
    }

    for (int32_t i = 0; i < textures.size(); i++) {
        std::string name = textures.at(i);
        ParseSpecTexName(name, wpmat, *pWPShaderInfo);
        material.textures.push_back(name);
        material.defines.push_back("g_Texture" + std::to_string(i));
        if (name.empty()) {
            continue;
        }

        std::array<uint16_t, 4> resolution;
        if (IsSpecTex(name)) {
            if (IsSpecLinkTex(name)) {
                svData.renderTargets.push_back({ i, name });
            } else if (pScene->renderTargets.count(name) == 0) {
                LOG_ERROR("%s not found in render targes", name.c_str());
            } else {
                svData.renderTargets.push_back({ i, name });
                const auto& rt = pScene->renderTargets.at(name);
                resolution     = { rt.width, rt.height, rt.width, rt.height };
            }
        } else {
            const ImageHeader& texh = texHeaders.count(name) == 0
                                          ? pScene->imageParser->ParseHeader(name)
                                          : texHeaders.at(name);
            if (i == 0) {
                if (texh.format == TextureFormat::R8)
                    pWPShaderInfo->combos["TEX0FORMAT"] = "FORMAT_R8";
                else if (texh.format == TextureFormat::RG8)
                    pWPShaderInfo->combos["TEX0FORMAT"] = "FORMAT_RG88";
            }
            if (texh.mipmap_larger) {
                resolution = { texh.width, texh.height, texh.mapWidth, texh.mapHeight };
            } else {
                resolution = { texh.mapWidth, texh.mapHeight, texh.mapWidth, texh.mapHeight };
            }

            if (pScene->textures.count(name) == 0) {
                SceneTexture stex;
                stex.sample = texh.sample;
                stex.url    = name;
                if (texh.isSprite) {
                    stex.isSprite   = texh.isSprite;
                    stex.spriteAnim = texh.spriteAnim;
                }
                pScene->textures[name] = stex;
            }
            if ((pScene->textures.at(name)).isSprite) {
                material.hasSprite = true;
                const auto& f1     = texh.spriteAnim.GetCurFrame();
                if (wpmat.shader == "genericparticle" || wpmat.shader == "genericropeparticle") {
                    pWPShaderInfo->combos["SPRITESHEET"] = "1";
                    pWPShaderInfo->combos["THICKFORMAT"] = "1";
                    if (algorism::IsPowOfTwo(texh.width) && algorism::IsPowOfTwo(texh.height)) {
                        pWPShaderInfo->combos["SPRITESHEETBLENDNPOT"] = "1";
                        resolution[2] = resolution[0] - resolution[0] % (int)f1.width;
                        resolution[3] = resolution[1] - resolution[1] % (int)f1.height;
                    }
                    materialShader.constValues["g_RenderVar1"] = std::array {
                        f1.xAxis[0], f1.yAxis[1], (float)(texh.spriteAnim.numFrames()), f1.rate
                    };
                }
            }
        }
        if (! resolution.empty()) {
            const std::string gResolution = WE_GLTEX_RESOLUTION_NAMES[i];

            materialShader.constValues[gResolution] = array_cast<float>(resolution);
        }
    }
    if (exists(pWPShaderInfo->combos, "LIGHTING")) {
        // pWPShaderInfo->combos["PRELIGHTING"] =
        // pWPShaderInfo->combos.at("LIGHTING");
    }

    if (! WPShaderParser::CompileToSpv(
            pScene->scene_id, sd_units, shader->codes, vfs, pWPShaderInfo, texinfos)) {
        return false;
    }

    material.blenmode = ParseBlendMode(wpmat.blending);

    for (uint i = 0; i < material.textures.size(); i++) {
        if (! exists(sd_units[1].preprocess_info.active_tex_slots, i)) material.textures[i].clear();
    }

    for (const auto& el : pWPShaderInfo->baseConstSvs) {
        materialShader.constValues[el.first] = el.second;
    }
    material.customShader = materialShader;
    material.name         = wpmat.shader;

    return true;
}

void LoadAlignment(SceneNode& node, std::string_view align, Vector2f size) {
    Vector3f trans = node.Translate();
    size *= 0.5f;
    size.y() *= 1.0f;

    auto contains = [&](std::string_view s) {
        return align.find(s) != std::string::npos;
    };

    // topleft top center ...
    if (contains("top")) trans.y() -= size.y();
    if (contains("left")) trans.x() += size.x();
    if (contains("right")) trans.x() -= size.x();
    if (contains("bottom")) trans.y() += size.y();

    node.SetTranslate(trans);
}

void LoadConstvalue(SceneMaterial& material, const wpscene::WPMaterial& wpmat,
                    const WPShaderInfo& info) {
    // load glname from alias and load to constvalue
    for (const auto& cs : wpmat.constantshadervalues) {
        const auto&               name  = cs.first;
        const std::vector<float>& value = cs.second;
        std::string               glname;
        if (info.alias.count(name) != 0) {
            glname = info.alias.at(name);
        } else {
            for (const auto& el : info.alias) {
                if (el.second.substr(2) == name) {
                    glname = el.second;
                    break;
                }
            }
        }
        if (glname.empty()) {
            LOG_ERROR("ShaderValue: %s not found in glsl", name.c_str());
        } else {
            material.customShader.constValues[glname] = value;
        }
    }
}

// parse

void ParseCamera(ParseContext& context, wpscene::WPSceneGeneral& general) {
    auto& scene = *context.scene;
    // effect camera
    scene.cameras["effect"]    = std::make_shared<SceneCamera>(2, 2, -1.0f, 1.0f);
    context.effect_camera_node = std::make_shared<SceneNode>(); // at 0,0,0
    scene.cameras.at("effect")->AttatchNode(context.effect_camera_node);
    scene.sceneGraph->AppendChild(context.effect_camera_node);

    // global camera
    scene.cameras["global"] = std::make_shared<SceneCamera>(int32_t(context.ortho_w / general.zoom),
                                                            int32_t(context.ortho_h / general.zoom),
                                                            -5000.0f,
                                                            5000.0f);
    scene.activeCamera      = scene.cameras.at("global").get();
    Vector3f cori { context.ortho_w / 2.0f, context.ortho_h / 2.0f, 0 },
        cscale { 1.0f, 1.0f, 1.0f }, cangle(Vector3f::Zero());

    context.global_camera_node = std::make_shared<SceneNode>(cori, cscale, cangle);
    scene.activeCamera->AttatchNode(context.global_camera_node);
    scene.sceneGraph->AppendChild(context.global_camera_node);

    scene.cameras["global_perspective"] =
        std::make_shared<SceneCamera>(context.ortho_w / (float)context.ortho_h,
                                      general.nearz,
                                      general.farz,
                                      algorism::CalculatePersperctiveFov(1000.0f, context.ortho_h));

    Vector3f cperori                       = cori;
    cperori[2]                             = 1000.0f;
    context.global_perspective_camera_node = std::make_shared<SceneNode>(cperori, cscale, cangle);
    scene.cameras["global_perspective"]->AttatchNode(context.global_perspective_camera_node);
    scene.sceneGraph->AppendChild(context.global_perspective_camera_node);
}

void InitContext(ParseContext& context, fs::VFS& vfs, wpscene::WPScene& sc) {
    context.scene            = std::make_shared<Scene>();
    context.vfs              = &vfs;
    auto& scene              = *context.scene;
    scene.imageParser        = std::make_unique<WPTexImageParser>(&vfs);
    scene.paritileSys->gener = std::make_unique<WPParticleRawGener>();
    scene.shaderValueUpdater = std::make_unique<WPShaderValueUpdater>(&scene);
    GenCardMesh(scene.default_effect_mesh, { 2, 2 });
    context.shader_updater = static_cast<WPShaderValueUpdater*>(scene.shaderValueUpdater.get());

    scene.clearColor = sc.general.clearcolor;
    scene.ortho[0]   = sc.general.orthogonalprojection.width;
    scene.ortho[1]   = sc.general.orthogonalprojection.height;
    context.ortho_w  = scene.ortho[0];
    context.ortho_h  = scene.ortho[1];

    {
        auto& gb              = context.global_base_uniforms;
        gb["g_ViewUp"]        = std::array { 0.0f, 1.0f, 0.0f };
        gb["g_ViewRight"]     = std::array { 1.0f, 0.0f, 0.0f };
        gb["g_ViewForward"]   = std::array { 0.0f, 0.0f, -1.0f };
        gb["g_EyePosition"]   = std::array { 0.0f, 0.0f, 0.0f };
        gb["g_TexelSize"]     = std::array { 1.0f / 1920.0f, 1.0f / 1080.0f };
        gb["g_TexelSizeHalf"] = std::array { 1.0f / 1920.0f / 2.0f, 1.0f / 1080.0f / 2.0f };

        gb["g_LightAmbientColor"] = sc.general.ambientcolor;
        gb["g_NormalModelMatrix"] = ShaderValue::fromMatrix(Matrix4f::Identity());
    }

    {
        WPCameraParallax cam_para;
        cam_para.enable         = sc.general.cameraparallax;
        cam_para.amount         = sc.general.cameraparallaxamount;
        cam_para.delay          = sc.general.cameraparallaxdelay;
        cam_para.mouseinfluence = sc.general.cameraparallaxmouseinfluence;
        context.shader_updater->SetCameraParallax(cam_para);
    }

    {
        auto ur          = std::make_shared<std::uniform_real_distribution<float>>(0.0f, 1.0f);
        auto randomSeed  = std::make_shared<std::default_random_engine>();
        context.randomFn = [randomSeed, ur]() {
            return (*ur)(*randomSeed);
        };
    }
}

void ParseImageObj(ParseContext& context, wpscene::WPImageObject& img_obj) {
    auto& wpimgobj = img_obj;
    if (! wpimgobj.visible) return;

    auto& vfs = *context.vfs;

    // coloBlendMode load passthrough manaully
    if (wpimgobj.colorBlendMode != 0) {
        wpscene::WPImageEffect colorEffect;
        wpscene::WPMaterial    colorMat;
        nlohmann::json         json;
        if (! PARSE_JSON(fs::GetFileContent(vfs, "/assets/materials/util/effectpassthrough.json"),
                         json))
            return;
        colorMat.FromJson(json);
        colorMat.combos["BONECOUNT"] = 1;
        colorMat.combos["BLENDMODE"] = wpimgobj.colorBlendMode;
        colorMat.blending            = "disabled";
        colorEffect.materials.push_back(colorMat);
        wpimgobj.effects.push_back(colorEffect);
    }

    int32_t count_eff = 0;
    for (const auto& wpeffobj : wpimgobj.effects) {
        if (wpeffobj.visible) count_eff++;
    }
    bool hasEffect = count_eff > 0;
    // skip no effect fullscreen layer
    if (! hasEffect && wpimgobj.fullscreen) return;

    bool hasPuppet = ! wpimgobj.puppet.empty();

    bool isCompose = (wpimgobj.image == "models/util/composelayer.json");
    // skip no effect compose layer
    // it's not the correct behaviour, but do it for now
    if (! hasEffect && isCompose) return;

    std::unique_ptr<WPMdl> puppet;
    if (! wpimgobj.puppet.empty()) {
        puppet = std::make_unique<WPMdl>();
        if (! WPMdlParser::Parse(wpimgobj.puppet, vfs, *puppet)) {
            LOG_ERROR("parse puppet failed: %s", wpimgobj.puppet.c_str());
            return;
        }
    }

    // wpimgobj.origin[1] = context.ortho_h - wpimgobj.origin[1];
    auto spImgNode = std::make_shared<SceneNode>(Vector3f(wpimgobj.origin.data()),
                                                 Vector3f(wpimgobj.scale.data()),
                                                 Vector3f(wpimgobj.angles.data()));
    LoadAlignment(*spImgNode, wpimgobj.alignment, { wpimgobj.size[0], wpimgobj.size[1] });
    spImgNode->ID() = wpimgobj.id;

    SceneMaterial     material;
    WPShaderValueData svData;

    ShaderValueMap baseConstSvs = context.global_base_uniforms;
    WPShaderInfo   shaderInfo;
    {
        if (! hasEffect) {
            svData.parallaxDepth = { wpimgobj.parallaxDepth[0], wpimgobj.parallaxDepth[1] };
            if (puppet) {
                WPMdlParser::AddPuppetShaderInfo(shaderInfo, *puppet);
            }
        }

        baseConstSvs["g_Alpha"]      = wpimgobj.alpha;
        baseConstSvs["g_Color"]      = wpimgobj.color;
        baseConstSvs["g_UserAlpha"]  = wpimgobj.alpha;
        baseConstSvs["g_Brightness"] = wpimgobj.brightness;

        shaderInfo.baseConstSvs = baseConstSvs;

        if (! LoadMaterial(vfs,
                           wpimgobj.material,
                           context.scene.get(),
                           spImgNode.get(),
                           &material,
                           &svData,
                           &shaderInfo)) {
            LOG_ERROR("load imageobj '%s' material faild", wpimgobj.name.c_str());
            return;
        };
        LoadConstvalue(material, wpimgobj.material, shaderInfo);
    }

    for (const auto& cs : wpimgobj.material.constantshadervalues) {
        const auto&               name  = cs.first;
        const std::vector<float>& value = cs.second;
        std::string               glname;
        if (shaderInfo.alias.count(name) != 0) {
            glname = shaderInfo.alias.at(name);
        } else {
            for (const auto& el : shaderInfo.alias) {
                if (el.second.substr(2) == name) {
                    glname = el.second;
                    break;
                }
            }
        }
        if (glname.empty()) {
            LOG_ERROR("ShaderValue: %s not found in glsl", name.c_str());
        } else {
            material.customShader.constValues[glname] = value;
        }
    }

    // mesh
    SceneMesh effct_final_mesh {};
    auto      spMesh = std::make_shared<SceneMesh>();
    auto&     mesh   = *spMesh;

    {
        // deal with pow of 2
        std::array<float, 2> mapRate { 1.0f, 1.0f };
        if (! wpimgobj.nopadding &&
            exists(material.customShader.constValues, WE_GLTEX_RESOLUTION_NAMES[0])) {
            const auto& r = material.customShader.constValues.at(WE_GLTEX_RESOLUTION_NAMES[0]);
            mapRate       = { r[2] / r[0], r[3] / r[1] };
        }

        if (puppet) {
            if (hasEffect) {
                GenCardMesh(
                    mesh, { (uint16_t)wpimgobj.size[0], (uint16_t)wpimgobj.size[1] }, mapRate);
                WPMdlParser::GenPuppetMesh(effct_final_mesh, *puppet);

                wpscene::WPImageEffect puppet_effect;
                wpscene::WPMaterial    puppet_mat;
                puppet_mat             = wpimgobj.material;
                puppet_mat.textures[0] = "";
                WPMdlParser::AddPuppetMatInfo(puppet_mat, *puppet);
                puppet_effect.materials.push_back(puppet_mat);
                wpimgobj.effects.push_back(puppet_effect);
            } else {
                svData.puppet_layer = WPPuppetLayer(puppet->puppet);
                svData.puppet_layer.prepared(wpimgobj.puppet_layers);
                WPMdlParser::GenPuppetMesh(mesh, *puppet);
            }
        }
        if (! puppet) {
            GenCardMesh(mesh, { (uint16_t)wpimgobj.size[0], (uint16_t)wpimgobj.size[1] }, mapRate);
            GenCardMesh(effct_final_mesh,
                        { (uint16_t)wpimgobj.size[0], (uint16_t)wpimgobj.size[1] });
        }
    }
    // material blendmode for last step to use
    auto imgBlendMode = material.blenmode;
    // disable img material blend, as it's the first effect node now
    if (hasEffect) {
        material.blenmode = BlendMode::Normal;
    }
    mesh.AddMaterial(std::move(material));
    spImgNode->AddMesh(spMesh);

    context.shader_updater->SetNodeData(spImgNode.get(), svData);
    if (hasEffect) {
        auto& scene = *context.scene;
        // currently use addr for unique
        std::string nodeAddr = getAddr(spImgNode.get());
        // set camera to attatch effect
        if (isCompose) {
            scene.cameras[nodeAddr] =
                std::make_shared<SceneCamera>((int32_t)scene.activeCamera->Width(),
                                              (int32_t)scene.activeCamera->Height(),
                                              -1.0f,
                                              1.0f);
            scene.cameras.at(nodeAddr)->AttatchNode(scene.activeCamera->GetAttachedNode());
            if (scene.linkedCameras.count("global") == 0) scene.linkedCameras["global"] = {};
            scene.linkedCameras.at("global").push_back(nodeAddr);
        } else {
            // applly scale to crop
            int32_t w               = wpimgobj.size[0];
            int32_t h               = wpimgobj.size[1];
            scene.cameras[nodeAddr] = std::make_shared<SceneCamera>(w, h, -1.0f, 1.0f);
            scene.cameras.at(nodeAddr)->AttatchNode(context.effect_camera_node);
        }
        spImgNode->SetCamera(nodeAddr);
        std::string effect_ppong_a, effect_ppong_b;
        effect_ppong_a = WE_EFFECT_PPONG_PREFIX_A.data() + nodeAddr;
        effect_ppong_b = WE_EFFECT_PPONG_PREFIX_B.data() + nodeAddr;
        // set image effect
        auto imgEffectLayer = std::make_shared<SceneImageEffectLayer>(
            spImgNode.get(), wpimgobj.size[0], wpimgobj.size[1], effect_ppong_a, effect_ppong_b);
        {
            imgEffectLayer->SetFinalBlend(imgBlendMode);
            imgEffectLayer->FinalMesh().ChangeMeshDataFrom(effct_final_mesh);
            imgEffectLayer->FinalNode().CopyTrans(*spImgNode);
            if (isCompose) {
            } else {
                spImgNode->CopyTrans(SceneNode());
            }
            scene.cameras.at(nodeAddr)->AttatchImgEffect(imgEffectLayer);
        }
        // set renderTarget for ping-pong operate
        {
            scene.renderTargets[effect_ppong_a] = { .width      = (uint16_t)wpimgobj.size[0],
                                                    .height     = (uint16_t)wpimgobj.size[1],
                                                    .allowReuse = true };
            if (wpimgobj.fullscreen) {
                scene.renderTargets[effect_ppong_a].bind = { .enable = true, .screen = true };
            }
            scene.renderTargets[effect_ppong_b] = scene.renderTargets.at(effect_ppong_a);
        }

        int32_t i_eff = -1;
        for (const auto& wpeffobj : wpimgobj.effects) {
            i_eff++;
            if (! wpeffobj.visible) {
                i_eff--;
                continue;
            }
            std::shared_ptr<SceneImageEffect> imgEffect = std::make_shared<SceneImageEffect>();

            // this will be replace when resolve, use here to get rt info
            const std::string inRT { effect_ppong_a };

            // fbo name map and effect command
            std::string effaddr = getAddr(imgEffectLayer.get());

            std::unordered_map<std::string, std::string> fboMap;
            {
                fboMap["previous"] = inRT;
                for (int32_t i = 0; i < wpeffobj.fbos.size(); i++) {
                    const auto& wpfbo  = wpeffobj.fbos.at(i);
                    std::string rtname = wpfbo.name + "_" + effaddr;
                    if (wpimgobj.fullscreen) {
                        scene.renderTargets[rtname]      = { 2, 2, true };
                        scene.renderTargets[rtname].bind = { .enable = true,
                                                             .screen = true,
                                                             .scale  = 1.0f / wpfbo.scale };
                    } else {
                        // i+2 for not override object's rt
                        scene.renderTargets[rtname] = {
                            .width      = (uint16_t)(wpimgobj.size[0] / wpfbo.scale),
                            .height     = (uint16_t)(wpimgobj.size[1] / wpfbo.scale),
                            .allowReuse = true
                        };
                    }
                    fboMap[wpfbo.name] = rtname;
                }
            }
            // load! effect commands
            {
                for (const auto& el : wpeffobj.commands) {
                    if (el.command != "copy") {
                        LOG_ERROR("Unknown effect command: %s", el.command.c_str());
                        continue;
                    }
                    if (fboMap.count(el.target) + fboMap.count(el.source) < 2) {
                        LOG_ERROR("Unknown effect command dst or src: %s %s",
                                  el.target.c_str(),
                                  el.source.c_str());
                        continue;
                    }
                    imgEffect->commands.push_back({ .cmd      = SceneImageEffect::CmdType::Copy,
                                                    .dst      = fboMap[el.target],
                                                    .src      = fboMap[el.source],
                                                    .afterpos = el.afterpos });
                }
            }

            bool eff_mat_ok { true };

            for (int32_t i_mat = 0; i_mat < wpeffobj.materials.size(); i_mat++) {
                wpscene::WPMaterial wpmat = wpeffobj.materials.at(i_mat);
                std::string         matOutRT { WE_EFFECT_PPONG_PREFIX_B };
                if (wpeffobj.passes.size() > i_mat) {
                    const auto& wppass = wpeffobj.passes.at(i_mat);
                    wpmat.MergePass(wppass);
                    // Set rendertarget, in and out
                    for (const auto& el : wppass.bind) {
                        if (fboMap.count(el.name) == 0) {
                            LOG_ERROR("fbo %s not found", el.name.c_str());
                            continue;
                        }
                        if (wpmat.textures.size() <= el.index) wpmat.textures.resize(el.index + 1);
                        wpmat.textures[el.index] = fboMap[el.name];
                    }
                    if (! wppass.target.empty()) {
                        if (fboMap.count(wppass.target) == 0) {
                            LOG_ERROR("fbo %s not found", wppass.target.c_str());
                        } else {
                            matOutRT = fboMap.at(wppass.target);
                        }
                    }
                }
                if (wpmat.textures.size() == 0) wpmat.textures.resize(1);
                if (wpmat.textures.at(0).empty()) {
                    wpmat.textures[0] = inRT;
                }
                auto         spEffNode  = std::make_shared<SceneNode>();
                std::string  effmataddr = getAddr(spEffNode.get());
                WPShaderInfo wpEffShaderInfo;
                wpEffShaderInfo.baseConstSvs = baseConstSvs;
                wpEffShaderInfo.baseConstSvs["g_EffectTextureProjectionMatrix"] =
                    ShaderValue::fromMatrix(Eigen::Matrix4f::Identity());
                wpEffShaderInfo.baseConstSvs["g_EffectTextureProjectionMatrixInverse"] =
                    ShaderValue::fromMatrix(Eigen::Matrix4f::Identity());
                SceneMaterial     material;
                WPShaderValueData svData;
                if (! LoadMaterial(vfs,
                                   wpmat,
                                   context.scene.get(),
                                   spEffNode.get(),
                                   &material,
                                   &svData,
                                   &wpEffShaderInfo)) {
                    eff_mat_ok = false;
                    break;
                }

                // load glname from alias and load to constvalue
                LoadConstvalue(material, wpmat, wpEffShaderInfo);
                auto spMesh = std::make_shared<SceneMesh>();
                {
                    svData.parallaxDepth = { wpimgobj.parallaxDepth[0], wpimgobj.parallaxDepth[1] };
                    if (puppet && wpmat.use_puppet) {
                        svData.puppet_layer = WPPuppetLayer(puppet->puppet);
                        svData.puppet_layer.prepared(wpimgobj.puppet_layers);
                    }
                }
                spMesh->AddMaterial(std::move(material));
                spEffNode->AddMesh(spMesh);

                context.shader_updater->SetNodeData(spEffNode.get(), svData);
                imgEffect->nodes.push_back({ matOutRT, spEffNode });
            }

            if (eff_mat_ok)
                imgEffectLayer->AddEffect(imgEffect);
            else {
                LOG_ERROR("effect \'%s\' failed to load", wpeffobj.name.c_str());
            }
        }
    }
    context.scene->sceneGraph->AppendChild(spImgNode);
}

void ParseParticleObj(ParseContext& context, wpscene::WPParticleObject& particle) {
    auto& wppartobj = particle;
    auto& vfs       = *context.vfs;
    if (wppartobj.particleObj.renderers.size() == 0) return;

    auto wppartRenderer = wppartobj.particleObj.renderers.at(0);
    bool render_rope    = sstart_with(wppartRenderer.name, "rope");
    bool hastrail       = send_with(wppartRenderer.name, "trail");

    if (render_rope) wppartobj.material.shader = "genericropeparticle";

    // wppartobj.origin[1] = context.ortho_h - wppartobj.origin[1];

    auto spNode = std::make_shared<SceneNode>(Vector3f(wppartobj.origin.data()),
                                              Vector3f(wppartobj.scale.data()),
                                              Vector3f(wppartobj.angles.data()));
    if (wppartobj.particleObj.flags[wpscene::Particle::FlagEnum::perspective]) {
        spNode->SetCamera("global_perspective");
    }

    SceneMaterial     material;
    WPShaderValueData svData;
    svData.parallaxDepth = { wppartobj.parallaxDepth[0], wppartobj.parallaxDepth[1] };
    WPShaderInfo shaderInfo;
    shaderInfo.baseConstSvs                         = context.global_base_uniforms;
    shaderInfo.baseConstSvs["g_OrientationUp"]      = std::array { 0.0f, 1.0f, 0.0f };
    shaderInfo.baseConstSvs["g_OrientationRight"]   = std::array { 1.0f, 0.0f, 0.0f };
    shaderInfo.baseConstSvs["g_OrientationForward"] = std::array { 0.0f, 0.0f, 1.0f };
    shaderInfo.baseConstSvs["g_ViewUp"]             = std::array { 0.0f, 1.0f, 0.0f };
    shaderInfo.baseConstSvs["g_ViewRight"]          = std::array { 1.0f, 0.0f, 0.0f };

    uint32_t maxcount = wppartobj.particleObj.maxcount;
    maxcount          = maxcount > 4000 ? 4000 : maxcount;

    if (hastrail) {
        float in_SegmentUVTimeOffset            = 0.0f;
        float in_SegmentMaxCount                = maxcount - 1;
        shaderInfo.baseConstSvs["g_RenderVar0"] = std::array { wppartRenderer.length,
                                                               wppartRenderer.maxlength,
                                                               in_SegmentUVTimeOffset,
                                                               in_SegmentMaxCount };
        shaderInfo.combos["THICKFORMAT"]        = "1";
        shaderInfo.combos["TRAILRENDERER"]      = "1";
    }

    if (! wppartobj.particleObj.flags[wpscene::Particle::FlagEnum::spritenoframeblending]) {
        shaderInfo.combos["SPRITESHEETBLEND"] = "1";
    }

    if (! LoadMaterial(vfs,
                       wppartobj.material,
                       context.scene.get(),
                       spNode.get(),
                       &material,
                       &svData,
                       &shaderInfo)) {
        LOG_ERROR("load particleobj '%s' material faild", wppartobj.name.c_str());
        return;
    }
    LoadConstvalue(material, wppartobj.material, shaderInfo);
    auto  spMesh             = std::make_shared<SceneMesh>(true);
    auto& mesh               = *spMesh;
    auto  animationmode      = ToAnimMode(wppartobj.particleObj.animationmode);
    auto  sequencemultiplier = wppartobj.particleObj.sequencemultiplier;
    bool  hasSprite          = material.hasSprite;

    bool thick_format = material.hasSprite || hastrail;
    if (render_rope)
        SetRopeParticleMesh(mesh, wppartobj.particleObj, maxcount, thick_format);
    else
        SetParticleMesh(mesh, wppartobj.particleObj, maxcount, thick_format);
    const auto& wpemitter   = wppartobj.particleObj.emitters[0];
    auto        particleSub = std::make_unique<ParticleSubSystem>(
        *context.scene->paritileSys,
        spMesh,
        maxcount,
        wppartobj.instanceoverride.rate,
        [=](const Particle& p, const ParticleRawGenSpec& spec) {
            auto& lifetime = *(spec.lifetime);
            if (lifetime < 0.0f) {
                lifetime = 0.0f;
                return;
            }
            switch (animationmode) {
            case ParticleAnimationMode::RANDOMONE: lifetime = std::floor(p.lifetimeInit); break;
            case ParticleAnimationMode::SEQUENCE:
                lifetime = (1.0f - (p.lifetime / p.lifetimeInit)) * sequencemultiplier;
                break;
            }
        });

    LoadEmitter(*particleSub,
                wppartobj.particleObj,
                wppartobj.instanceoverride.count,
                context.randomFn,
                render_rope);
    LoadInitializer(
        *particleSub, wppartobj.particleObj, wppartobj.instanceoverride, context.randomFn);
    LoadOperator(*particleSub, wppartobj.particleObj, wppartobj.instanceoverride, context.randomFn);

    context.scene->paritileSys->subsystems.emplace_back(std::move(particleSub));
    mesh.AddMaterial(std::move(material));
    spNode->AddMesh(spMesh);
    context.shader_updater->SetNodeData(spNode.get(), svData);
    context.scene->sceneGraph->AppendChild(spNode);
}

void ParseLightObj(ParseContext& context, wpscene::WPLightObject& light_obj) {
    auto node = std::make_shared<SceneNode>(Vector3f(light_obj.origin.data()),
                                            Vector3f(light_obj.scale.data()),
                                            Vector3f(light_obj.angles.data()));

    context.scene->lights.emplace_back(std::make_unique<SceneLight>(
        Vector3f(light_obj.color.data()), light_obj.radius, light_obj.intensity));

    auto& light = *(context.scene->lights.back());
    light.setNode(node);

    context.scene->sceneGraph->AppendChild(node);
}
} // namespace

std::shared_ptr<Scene> WPSceneParser::Parse(std::string_view scene_id, const std::string& buf,
                                            fs::VFS& vfs, audio::SoundManager& sm) {
    nlohmann::json json;
    if (! PARSE_JSON(buf, json)) return nullptr;
    wpscene::WPScene sc;
    sc.FromJson(json);
    //	LOG_INFO(nlohmann::json(sc).dump(4));

    ParseContext context;

    std::vector<WPObjectVar> wp_objs;

    for (auto& obj : json.at("objects")) {
        if (obj.contains("image") && ! obj.at("image").is_null()) {
            wpscene::WPImageObject wpimgobj;
            if (! wpimgobj.FromJson(obj, vfs)) continue;
            if (! wpimgobj.visible) continue;
            wp_objs.push_back(wpimgobj);
        } else if (obj.contains("particle") && ! obj.at("particle").is_null()) {
            wpscene::WPParticleObject wppartobj;
            if (! wppartobj.FromJson(obj, vfs)) continue;
            if (! wppartobj.visible) continue;
            wp_objs.push_back(wppartobj);
        } else if (obj.contains("sound") && ! obj.at("sound").is_null()) {
            wpscene::WPSoundObject wpsoundobj;
            if (! wpsoundobj.FromJson(obj)) continue;
            wp_objs.push_back(wpsoundobj);
        } else if (obj.contains("light") && ! obj.at("light").is_null()) {
            wpscene::WPLightObject wplightobj;
            if (! wplightobj.FromJson(obj)) continue;
            if (! wplightobj.visible) continue;
            wp_objs.push_back(wplightobj);
        }
    }

    if (sc.general.orthogonalprojection.auto_) {
        uint32_t w = 0, h = 0;
        for (auto& obj : wp_objs) {
            auto* img = std::get_if<wpscene::WPImageObject>(&obj);
            if (img == nullptr) continue;
            uint32_t size = img->size.at(0) * img->size.at(1);
            if (size > w * h) {
                w = img->size.at(0);
                h = img->size.at(1);
            }
        }
        sc.general.orthogonalprojection.width  = w;
        sc.general.orthogonalprojection.height = h;
    }

    InitContext(context, vfs, sc);
    ParseCamera(context, sc.general);

    {
        context.scene->renderTargets[SpecTex_Default.data()] = { .width  = context.ortho_w,
                                                                 .height = context.ortho_w,
                                                                 .bind   = { .enable = true,
                                                                             .screen = true } };
        context.scene->renderTargets[WE_MIP_MAPPED_FRAME_BUFFER.data()] = {
            .width      = context.ortho_w,
            .height     = context.ortho_w,
            .has_mipmap = true,
            .bind       = { .enable = true, .name = SpecTex_Default.data() }
        };
    }

    context.scene->scene_id = scene_id;

    WPShaderParser::InitGlslang();

    for (WPObjectVar& obj : wp_objs) {
        std::visit(visitor::overload {
                       [&context](wpscene::WPImageObject& obj) {
                           ParseImageObj(context, obj);
                       },
                       [&context](wpscene::WPParticleObject& obj) {
                           ParseParticleObj(context, obj);
                       },
                       [&context, &sm](wpscene::WPSoundObject& obj) {
                           WPSoundParser::Parse(obj, *context.vfs, sm);
                       },
                       [&context](wpscene::WPLightObject& obj) {
                           ParseLightObj(context, obj);
                       },
                   },
                   obj);
    }

    WPShaderParser::FinalGlslang();
    return context.scene;
}
