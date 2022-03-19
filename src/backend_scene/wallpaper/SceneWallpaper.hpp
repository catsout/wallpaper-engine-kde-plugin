#pragma once
#include <memory>
#include <string_view>
#include "Type.h"
#include "Utils/span.hpp"
#include "Swapchain/ExSwapchain.hpp"

namespace wallpaper
{

constexpr std::string_view PROPERTY_SOURCE   = "source";
constexpr std::string_view PROPERTY_ASSETS   = "assets";
constexpr std::string_view PROPERTY_FPS      = "fps";
constexpr std::string_view PROPERTY_FILLMODE = "fillmode";
constexpr std::string_view PROPERTY_GRAPHIVZ = "graphivz";

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

    void setPropertyBool(std::string_view, bool);
    void setPropertyInt32(std::string_view, int32_t);
    void setPropertyFloat(std::string_view, float);
    void setPropertyString(std::string_view, std::string);
    
    ExSwapchain* exSwapchain() const;

private:
    bool m_inited {false};
private:
    friend class MainHandler;
    /*
    class impl;
    std::unique_ptr<impl> pImpl;
    */
    bool m_offscreen {false};
    std::shared_ptr<MainHandler> m_main_handler;
};
}