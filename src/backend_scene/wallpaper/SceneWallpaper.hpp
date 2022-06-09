#pragma once
#include <memory>
#include <string_view>
#include "Type.hpp"
#include "Utils/span.hpp"
#include "Swapchain/ExSwapchain.hpp"

namespace wallpaper
{

constexpr std::string_view PROPERTY_SOURCE     = "source";
constexpr std::string_view PROPERTY_ASSETS     = "assets";
constexpr std::string_view PROPERTY_FPS        = "fps";
constexpr std::string_view PROPERTY_FILLMODE   = "fillmode";
constexpr std::string_view PROPERTY_GRAPHIVZ   = "graphivz";
constexpr std::string_view PROPERTY_VOLUME     = "volume";
constexpr std::string_view PROPERTY_MUTED      = "muted";
constexpr std::string_view PROPERTY_CACHE_PATH = "cache_path";

#include "Utils/NoCopyMove.hpp"
class MainHandler;
struct RenderInitInfo;

class SceneWallpaper : NoCopy {
public:
    SceneWallpaper();
    ~SceneWallpaper();
    bool init();
    bool inited() const;

    void initVulkan(const RenderInitInfo&);

    void play();
    void pause();
    void mouseInput(double x, double y);

    void setPropertyBool(std::string_view, bool);
    void setPropertyInt32(std::string_view, int32_t);
    void setPropertyFloat(std::string_view, float);
    void setPropertyString(std::string_view, std::string);

    ExSwapchain* exSwapchain() const;

private:
    bool m_inited { false };

private:
    friend class MainHandler;

    bool                         m_offscreen { false };
    std::shared_ptr<MainHandler> m_main_handler;
};
} // namespace wallpaper
