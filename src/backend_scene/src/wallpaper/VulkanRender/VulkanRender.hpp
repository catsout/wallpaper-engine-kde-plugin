#pragma once

#include "Vulkan/Device.hpp"
#include "Vulkan/TextureCache.hpp"
#include "Vulkan/Swapchain.hpp"
#include "Vulkan/VulkanExSwapchain.hpp"

#include "Resource.hpp"

#include "RenderGraph/RenderGraph.hpp"

#include "SceneWallpaperSurface.hpp"
#include "VulkanPass.hpp"
#include "PrePass.hpp"
#include "FinPass.hpp"
#include "Resource.hpp"

#include <cstdio>
#include <memory>

namespace wallpaper
{
class Scene;

namespace vulkan
{
class FinPass;

class VulkanRender {
public:

    bool init(Span<std::uint8_t> uuid);
    bool init(VulkanSurfaceInfo&);

    void destroy();

    void drawFrame(Scene&);
    void compileRenderGraph(Scene&, rg::RenderGraph&);

	bool CreateRenderingResource(RenderingResources&);
	void DestroyRenderingResource(RenderingResources&);

private:
    bool initRes();
    void drawFrameSwapchain();
    void drawFrameOffscreen();

    Instance m_instance;
    std::unique_ptr<PrePass> m_prepass {nullptr};
    std::unique_ptr<FinPass> m_finpass {nullptr};

    std::unique_ptr<StagingBuffer> m_vertex_buf {nullptr};
    std::unique_ptr<StagingBuffer> m_ubo_buf {nullptr};
    vk::CommandBuffer m_upload_cmd;

    std::unique_ptr<Device> m_device;

    bool m_with_surface {false};
    bool m_inited       {false};
    bool m_pass_loaded  {false};

    std::unique_ptr<VulkanExSwapchain> m_ex_swapchain;
    std::array<RenderingResources, 3> m_rendering_resources;

    std::vector<VulkanPass*> m_passes;
};
}
}