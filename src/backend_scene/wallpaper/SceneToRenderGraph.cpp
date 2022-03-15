#include "SceneToRenderGraph.hpp"

#include "Scene/Scene.h"
#include "RenderGraph/RenderGraph.hpp"
#include "SpecTexs.h"
#include "Utils/Logging.h"
#include "Utils/MapSet.hpp"

#include "VulkanRender/AllPasses.hpp"

using namespace wallpaper;
namespace wallpaper::rg
{

static void doCopy(RenderGraphBuilder& builder, vulkan::CopyPass::Desc& desc, TexNode* in, TexNode* out) {
    builder.read(in);
    builder.write(out);

    desc.src = in->key();
    desc.dst = out->key();
}
static void addCopyPass(RenderGraph& rgraph, TexNode* in, TexNode* out) {
    rgraph.addPass<vulkan::CopyPass>("copy", PassNode::Type::Copy,
        [&in, &out](RenderGraphBuilder& builder, vulkan::CopyPass::Desc& desc) {
            doCopy(builder, desc, in, out);
    });
}

static void addCopyPass(RenderGraph& rgraph, const TexNode::Desc& in, const TexNode::Desc& out) {
    rgraph.addPass<vulkan::CopyPass>("copy", PassNode::Type::Copy,
        [&in, &out](RenderGraphBuilder& builder, vulkan::CopyPass::Desc& desc) {
            auto* in_node = builder.createTexNode(in);
            auto* out_node = builder.createTexNode(out, true);
            doCopy(builder, desc, in_node, out_node);
    });
}

static TexNode* addCopyPass(RenderGraph& rgraph, TexNode* in) {
    TexNode* copy {nullptr};
    rgraph.addPass<vulkan::CopyPass>("copy", PassNode::Type::Copy,
        [&copy, in](RenderGraphBuilder& builder, vulkan::CopyPass::Desc& pdesc) {
            auto desc = in->genDesc();
            desc.key += "_copy";
            desc.name += "_copy";
            copy = builder.createTexNode(desc, true);
            doCopy(builder, pdesc, in, copy);
        });
    return copy;
}

static TexNode::Desc createTexDesc(std::string path) {
    return TexNode::Desc{
        .name = path,
        .key = path,
        .type = IsSpecTex(path) ? TexNode::TexType::Temp : TexNode::TexType::Imported
    };
}
}

static void TraverseNode(const std::function<void(SceneNode*)>& func, SceneNode* node) {
	func(node);
	for(auto& child:node->GetChildren())
		TraverseNode(func, child.get());
}

static void CheckAndSetSprite(Scene& scene, vulkan::CustomShaderPass::Desc& desc, Span<std::string> texs) {
    for(uint i=0;i<texs.size();i++) {
        auto& tex = texs[i];
        if(!tex.empty() && !IsSpecTex(tex) && scene.textures.count(tex) != 0) {
            const auto& stex = scene.textures.at(tex);
            if(stex.isSprite) {
                desc.sprites_map[i] = stex.spriteAnim;
            }
        }
    }
}

struct ExtraInfo {
    Map<size_t, rg::TexNode*> idMap;
    rg::RenderGraph* rgraph;
    Scene* scene;
};

static void ToGraphPass(SceneNode* node, std::string output, uint32_t imgId, ExtraInfo& extra) {
    auto& rgraph = *extra.rgraph;
    auto& scene = *extra.scene;

	auto loadEffect = [node, &rgraph, &scene, &extra](SceneImageEffectLayer* effs) {
        effs->ResolveEffect(scene.default_effect_mesh, "effect");

		for(int32_t i=0;i<effs->EffectCount();i++) {
			auto& eff = effs->GetEffect(i);
			auto cmdItor = eff->commands.begin();
			auto cmdEnd = eff->commands.end();
			int nodePos = 0;
			for(auto& n:eff->nodes) {
				if(cmdItor != cmdEnd && nodePos == cmdItor->afterpos) {
					//AddCopyCmdPasses(cmdItor->dst, cmdItor->src);
                    rg::addCopyPass(rgraph, rg::createTexDesc(cmdItor->dst), rg::createTexDesc(cmdItor->src));
					cmdItor++;
				}
				auto& name = n.output;
				ToGraphPass(n.sceneNode.get(), name, node->ID(), extra);
				nodePos++;
			}
		}
	};

	if(node->Mesh() == nullptr) return;
	auto* mesh = node->Mesh();
	if(mesh->Material() == nullptr) return;
	auto* material = mesh->Material();
	auto* mshaderPtr = material->customShader.shader.get();

	SceneImageEffectLayer* imgeff = nullptr;
	if(!node->Camera().empty()) {
		auto& cam = scene.cameras.at(node->Camera());
		if(cam->HasImgEffect()) {
			imgeff = cam->GetImgEffect().get();
			output = imgeff->FirstTarget();
		}
	}

    std::string passName = material->name;

    rgraph.addPass<vulkan::CustomShaderPass>(passName, rg::PassNode::Type::CustomShader,
        [material, node, &output, &imgId, &rgraph, &scene, &extra](rg::RenderGraphBuilder& builder, vulkan::CustomShaderPass::Desc& pdesc) {
            const auto& pass = builder.workPassNode();
            pdesc.node = node;
            pdesc.output = output;
            CheckAndSetSprite(scene, pdesc, material->textures);
            for(const auto& url:material->textures) {
                rg::TexNode* input {nullptr};
                if(url.empty()) {
                    pdesc.textures.emplace_back("");
                    continue;
                } else {
                    rg::TexNode::Desc desc;
                    desc.key = url;
                    desc.name = url;
                    desc.type = !IsSpecTex(url)
                        ? rg::TexNode::TexType::Imported
                        : rg::TexNode::TexType::Temp;
                    input = builder.createTexNode(desc);
                }

                if(url == output) {
                    input = rg::addCopyPass(rgraph, input);
                } else if(IsSpecLinkTex(url)) {
                    auto id = ParseLinkTex(url);
                    if(extra.idMap.count(id)) {
                        //rg::addCopyPass(rgraph, extra.idMap.at(id), input);
                    }
                }
                builder.read(input);
                pdesc.textures.emplace_back(input->key());
            }

            rg::TexNode* output_node {nullptr};
            output_node = builder.createTexNode(rg::TexNode::Desc {
                .name = output,
                .key = output,
                .type = rg::TexNode::TexType::Temp
            }, true);
            builder.write(output_node);
      		if(output == SpecTex_Default) {
				extra.idMap[imgId] = output_node;
			}
        });

    // load effect
	if(imgeff != nullptr) loadEffect(imgeff);
}



std::unique_ptr<rg::RenderGraph> wallpaper::sceneToRenderGraph(Scene& scene) {
    using namespace std::placeholders;

    std::unique_ptr<rg::RenderGraph> rgraph = std::make_unique<rg::RenderGraph>();
    ExtraInfo extra {
        .rgraph = rgraph.get(),
        .scene = &scene
    };
    TraverseNode(std::bind(&ToGraphPass, _1, std::string(SpecTex_Default), 0, extra), scene.sceneGraph.get()); 
    return rgraph;
}