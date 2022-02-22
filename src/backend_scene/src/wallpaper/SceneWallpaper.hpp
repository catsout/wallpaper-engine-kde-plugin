#pragma once
#include <memory>
#include <string_view>
#include "Utils/span.hpp"

namespace wallpaper
{

#include "Utils/NoCopyMove.hpp"
class MainHandler;
class RenderHandler;
struct VulkanSurfaceInfo;

class SceneWallpaper : NoCopy {
public:
    SceneWallpaper();
    ~SceneWallpaper();
    bool init();
    bool inited() const;

    void initVulkan(const std::shared_ptr<VulkanSurfaceInfo>&);
    void initVulkanEx(Span<uint8_t> uuid);
    void draw();

    void setProperty(std::string_view name, std::string_view value);

private:
    bool m_inited {false};
    bool m_exswap_mode {false};
private:
    friend class MainHandler;
    friend class RenderHandler;
    class impl;
    std::unique_ptr<impl> pImpl;
};
}