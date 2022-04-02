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
#include "Type.hpp"

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
    bool init(RenderInitInfo);

    void destroy();

    void drawFrame(Scene&);

    void clearLastRenderGraph();
    void compileRenderGraph(Scene&, rg::RenderGraph&);
    void UpdateCameraFillMode(Scene&, wallpaper::FillMode);

    bool CreateRenderingResource(RenderingResources&);
    void DestroyRenderingResource(RenderingResources&);

    VulkanExSwapchain* exSwapchain() const;
    bool               inited() const;

private:
    bool initRes();
    void drawFrameSwapchain();
    void drawFrameOffscreen();
    void setRenderTargetSize(Scene&, rg::RenderGraph&);

    Instance                 m_instance;
    std::unique_ptr<PrePass> m_prepass { nullptr };
    std::unique_ptr<FinPass> m_finpass { nullptr };

    std::unique_ptr<FinPass> m_testpass { nullptr };
    ReDrawCB                 m_redraw_cb;

    std::unique_ptr<StagingBuffer> m_vertex_buf { nullptr };
    std::unique_ptr<StagingBuffer> m_dyn_buf { nullptr };
    vk::CommandBuffer              m_upload_cmd;

    std::unique_ptr<Device> m_device;

    bool m_with_surface { false };
    bool m_inited { false };
    bool m_pass_loaded { false };

    std::unique_ptr<VulkanExSwapchain> m_ex_swapchain;
    std::array<RenderingResources, 3>  m_rendering_resources;

    std::vector<VulkanPass*> m_passes;
};
} // namespace vulkan
} // namespace wallpaper