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

static TexNode* addCopyPass(RenderGraph& rgraph, TexNode* in, TexNode::Desc* out_desc=nullptr) {
    TexNode* copy {nullptr};
    rgraph.addPass<vulkan::CopyPass>("copy", PassNode::Type::Copy,
        [&copy, in, out_desc](RenderGraphBuilder& builder, vulkan::CopyPass::Desc& pdesc) {
            auto desc = out_desc == nullptr ? in->genDesc() : *out_desc;
            if(out_desc == nullptr) {
                desc.key += "_" + std::to_string(in->version()) + "_copy";
                desc.name += "_" + std::to_string(in->version()) + "_copy";
            }
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

struct DelayLinkInfo {
    rg::NodeID id;
    rg::NodeID link_id;
    size_t tex_index;
};

struct ExtraInfo {
    Map<size_t, rg::TexNode*> id_link_map {};
    std::vector<DelayLinkInfo> link_info {};
    rg::RenderGraph* rgraph {nullptr};
    Scene* scene {nullptr};
    bool use_mipmap_framebuffer {false};
};

static void ToGraphPass(SceneNode* node, std::string_view output, uint32_t imgId, ExtraInfo& extra) {
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
            for(uint i=0;i<material->textures.size();i++) {
                const auto& url = material->textures[i];
                rg::TexNode* input {nullptr};
                if(url.empty()) {
                    pdesc.textures.emplace_back("");
                    continue;
                } else if(IsSpecLinkTex(url)) {
                    auto id = ParseLinkTex(url);
                    extra.link_info.push_back(DelayLinkInfo {
                        .id = pass.ID(),
                        .link_id = id,
                        .tex_index = i
                    });
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
                    if(sstart_with(url, WE_MIP_MAPPED_FRAME_BUFFER)) {
                        extra.use_mipmap_framebuffer = true;
                        builder.markVirtualWrite(input);
                    }
                }

                if(url == output) {
                    builder.markSelfWrite(input);
                    input = rg::addCopyPass(rgraph, input);
                } 
                builder.read(input);
                pdesc.textures.emplace_back(input->key());
            }

            rg::TexNode* output_node {nullptr};
            output_node = builder.createTexNode(rg::TexNode::Desc {
                .name = output.data(),
                .key = output.data(),
                .type = rg::TexNode::TexType::Temp
            }, true);
            builder.write(output_node);
      		if(output == SpecTex_Default) {
				extra.id_link_map[imgId] = output_node;
			}
        });

    // load effect
	if(imgeff != nullptr) loadEffect(imgeff);
}



std::unique_ptr<rg::RenderGraph> wallpaper::sceneToRenderGraph(Scene& scene) {
    std::unique_ptr<rg::RenderGraph> rgraph = std::make_unique<rg::RenderGraph>();
    ExtraInfo extra {
        .rgraph = rgraph.get(),
        .scene = &scene
    };
    TraverseNode([&extra](SceneNode* node) {
        ToGraphPass(node, SpecTex_Default, node->ID(), extra);
    }, scene.sceneGraph.get()); 

    for(auto& info:extra.link_info) {
        if(!exists(extra.id_link_map, info.link_id)) {
            LOG_ERROR("link tex %d not found", info.link_id);
            continue;
        }
        rgraph->afterBuild(info.id,[&rgraph, &extra, &info](rg::RenderGraphBuilder& builder, rg::Pass& rgpass) {
            auto& pass = static_cast<vulkan::CustomShaderPass&>(rgpass);

            auto* link_tex_node = extra.id_link_map.at(info.link_id);
            auto copy_desc = link_tex_node->genDesc();
            copy_desc.key = GenLinkTex(info.link_id);
            copy_desc.name = copy_desc.key;

            auto new_in = rg::addCopyPass(*rgraph, link_tex_node, &copy_desc);
            builder.read(new_in);
            pass.setDescTex(info.tex_index, new_in->key());
            return true;
        });
    }

    if(extra.use_mipmap_framebuffer) {
        rg::addCopyPass(*rgraph, rg::TexNode::Desc {
            .name = SpecTex_Default.data(),
            .key = SpecTex_Default.data(),
            .type = rg::TexNode::TexType::Temp
        }, rg::TexNode::Desc {
            .name = WE_MIP_MAPPED_FRAME_BUFFER.data(),
            .key = WE_MIP_MAPPED_FRAME_BUFFER.data(),
            .type = rg::TexNode::TexType::Temp
        });
    }

    return rgraph;
}