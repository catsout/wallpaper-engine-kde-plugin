#include "SceneWallpaper.hpp"
#include "SceneWallpaperSurface.hpp"

#include "Utils/Logging.h"
#include "Looper/Looper.hpp"

#include "Utils/FrameTimer.h"
#include "Utils/FpsCounter.h"
#include "WPSceneParser.hpp"
#include "Scene/Scene.h"

#include "Fs/VFS.h"
#include "Fs/PhysicalFs.h"
#include "WPPkgFs.hpp"

#include "Audio/SoundManager.h"

#include "RenderGraph/RenderGraph.hpp"

#include "VulkanRender/SceneToRenderGraph.hpp"
#include "VulkanRender/VulkanRender.hpp"
#include <atomic>

using namespace wallpaper;

#define CASE_CMD(cmd)      \
    case CMD::CMD_##cmd:   \
        handle_##cmd(msg); \
        break;
#define MHANDLER_CMD(cmd) void handle_##cmd(const std::shared_ptr<looper::Message>& msg)
#define MHANDLER_CMD_IMPL(cl, cmd) \
    void impl_##cl::handle_##cmd(const std::shared_ptr<looper::Message>& msg)
#define CALL_MHANDLER_CMD(cmd, msg) handle_##cmd(msg)

template<typename T>
static void addMsgCmd(looper::Message& msg, T cmd) {
    msg.setInt32("cmd", (int32_t)cmd);
}

namespace wallpaper
{
class RenderHandler;

class MainHandler : public looper::Handler {
public:
    enum class CMD
    {
        CMD_LOAD_SCENE,
        CMD_SET_PROPERTY,
        CMD_STOP,
        CMD_NO
    };

public:
    MainHandler();
    virtual ~MainHandler() {};

    bool init();
    auto renderHandler() const { return m_render_handler; }
    bool inited() const { return m_inited; }

public:
    void onMessageReceived(const std::shared_ptr<looper::Message>& msg) override {
        int32_t cmd_int = (int32_t)CMD::CMD_NO;
        if (msg->findInt32("cmd", &cmd_int)) {
            CMD cmd = static_cast<CMD>(cmd_int);
            switch (cmd) {
                CASE_CMD(SET_PROPERTY);
                CASE_CMD(LOAD_SCENE);
                CASE_CMD(STOP);
            default: break;
            }
        }
    }

    void sendCmdLoadScene();
    bool isGenGraphviz() const { return m_gen_graphviz; }

private:
    void loadScene();

    MHANDLER_CMD(LOAD_SCENE);
    MHANDLER_CMD(SET_PROPERTY);
    MHANDLER_CMD(STOP);

private:
    bool m_inited { false };

    std::string m_assets;
    std::string m_source;
    std::string m_cache_path;
    bool        m_gen_graphviz { false };

    WPSceneParser                        m_scene_parser;
    std::unique_ptr<audio::SoundManager> m_sound_manager;

private:
    std::shared_ptr<looper::Looper> m_main_loop;
    std::shared_ptr<looper::Looper> m_render_loop;
    std::shared_ptr<RenderHandler>  m_render_handler;
};
// for macro
using impl_MainHandler = MainHandler;

class RenderHandler : public looper::Handler {
public:
    enum class CMD
    {
        CMD_INIT_VULKAN,
        CMD_SET_SCENE,
        CMD_SET_FILLMODE,
        CMD_STOP,
        CMD_DRAW,
        CMD_NO
    };
    MainHandler& main_handler;
    RenderHandler(MainHandler& m)
        : main_handler(m), m_render(std::make_unique<vulkan::VulkanRender>()) {}
    virtual ~RenderHandler() {
        frame_timer.Stop();
        m_render->destroy();
        LOG_INFO("render handler deleted");
    }

    void onMessageReceived(const std::shared_ptr<looper::Message>& msg) override {
        int32_t cmd_int = (int32_t)CMD::CMD_NO;
        if (msg->findInt32("cmd", &cmd_int)) {
            CMD cmd = static_cast<CMD>(cmd_int);
            switch (cmd) {
                CASE_CMD(DRAW);
                CASE_CMD(STOP);
                CASE_CMD(SET_FILLMODE);
                CASE_CMD(SET_SCENE);
                CASE_CMD(INIT_VULKAN);
            }
        }
    }

    ExSwapchain* exSwapchain() const { return m_render->exSwapchain(); }

    bool renderInited() const { return m_render->inited(); }

    void setMousePos(float x, float y) { m_mouse_pos.store(std::array { x, y }); }

private:
    MHANDLER_CMD(STOP) {
        bool stop { false };
        if (msg->findBool("value", &stop)) {
            if (stop)
                frame_timer.Stop();
            else
                frame_timer.Run();
        }
    }
    MHANDLER_CMD(DRAW) {
        frame_timer.FrameBegin();
        if (m_rg) {
            // LOG_INFO("frame info, fps: %.1f, frametime: %.1f", 1.0f, 1000.0f*m_scene->frameTime);
            m_scene->shaderValueUpdater->FrameBegin();
            {
                auto pos = m_mouse_pos.load();
                m_scene->shaderValueUpdater->MouseInput(pos[0], pos[1]);
            }
            m_scene->paritileSys.Emitt();

            m_render->drawFrame(*m_scene);

            m_scene->PassFrameTime(frame_timer.IdeaTime());

            m_scene->shaderValueUpdater->FrameEnd();
            // fps_counter.RegisterFrame();
        }
        frame_timer.FrameEnd();
    }
    MHANDLER_CMD(SET_FILLMODE) {
        int32_t value;
        if (msg->findInt32("value", &value)) {
            m_fillmode = (FillMode)value;
            if (m_scene && renderInited()) {
                m_render->UpdateCameraFillMode(*m_scene, m_fillmode);
            }
        }
    }
    MHANDLER_CMD(SET_SCENE) {
        if (msg->findObject("scene", &m_scene)) {
            decltype(m_rg) old_rg = std::move(m_rg);
            m_rg                  = sceneToRenderGraph(*m_scene);

            if (main_handler.isGenGraphviz()) m_rg->ToGraphviz("graph.dot");
            m_render->compileRenderGraph(*m_scene, *m_rg);
            m_render->UpdateCameraFillMode(*m_scene, m_fillmode);
        }
    }
    MHANDLER_CMD(INIT_VULKAN) {
        std::shared_ptr<RenderInitInfo> info;
        if (msg->findObject("info", &info)) {
            m_render->init(*info);

            // inited, callback to laod scene
            main_handler.sendCmdLoadScene();
        }
    }

public:
    FrameTimer frame_timer;
    FpsCounter fps_counter;
    ;

private:
    std::shared_ptr<Scene>                m_scene { nullptr };
    std::unique_ptr<rg::RenderGraph>      m_rg { nullptr };
    std::unique_ptr<vulkan::VulkanRender> m_render;
    FillMode                              m_fillmode { FillMode::ASPECTCROP };

    std::atomic<std::array<float, 2>> m_mouse_pos { std::array { 0.5f, 0.5f } };
};
} // namespace wallpaper

SceneWallpaper::SceneWallpaper(): m_main_handler(std::make_shared<MainHandler>()) {}

SceneWallpaper::~SceneWallpaper() {
    /*
    if(m_offscreen) {
        // no wait
        auto msg = looper::Message::create(0, m_main_handler);
        msg->setObject("self_clean", m_main_handler);
        msg->setCleanAfterDeliver(true);
        m_main_handler = nullptr;
        msg->post();
    }
    */
}

bool SceneWallpaper::inited() const { return m_main_handler->inited(); }

bool SceneWallpaper::init() { return m_main_handler->init(); }

void SceneWallpaper::initVulkan(const RenderInitInfo& info) {
    m_offscreen                             = info.offscreen;
    std::shared_ptr<RenderInitInfo> sp_info = std::make_shared<RenderInitInfo>(info);
    auto msg = looper::Message::create(0, m_main_handler->renderHandler());
    addMsgCmd(*msg, RenderHandler::CMD::CMD_INIT_VULKAN);
    msg->setObject("info", sp_info);
    msg->post();
}

void SceneWallpaper::play() {
    auto msg = looper::Message::create(0, m_main_handler);
    addMsgCmd(*msg, MainHandler::CMD::CMD_STOP);
    msg->setBool("value", false);
    msg->post();
}
void SceneWallpaper::pause() {
    auto msg = looper::Message::create(0, m_main_handler);
    addMsgCmd(*msg, MainHandler::CMD::CMD_STOP);
    msg->setBool("value", true);
    msg->post();
}

void SceneWallpaper::mouseInput(double x, double y) {
    m_main_handler->renderHandler()->setMousePos(x, y);
}

#define BASIC_TYPE(NAME, TYPENAME)                                                  \
    void SceneWallpaper::setProperty##NAME(std::string_view name, TYPENAME value) { \
        auto msg = looper::Message::create(0, m_main_handler);                      \
        addMsgCmd(*msg, MainHandler::CMD::CMD_SET_PROPERTY);                        \
        msg->setString("property", std::string(name));                              \
        msg->set##NAME("value", value);                                             \
        msg->post();                                                                \
    }

BASIC_TYPE(Bool, bool);
BASIC_TYPE(Int32, int32_t);
BASIC_TYPE(Float, float);
BASIC_TYPE(String, std::string);

ExSwapchain* SceneWallpaper::exSwapchain() const {
    return m_main_handler->renderHandler()->exSwapchain();
}

MHANDLER_CMD_IMPL(MainHandler, LOAD_SCENE) {
    if (m_render_handler->renderInited()) {
        loadScene();
    }
}

MHANDLER_CMD_IMPL(MainHandler, SET_PROPERTY) {
    std::string property;
    if (msg->findString("property", &property)) {
        if (property == PROPERTY_SOURCE) {
            msg->findString("value", &m_source);
            LOG_INFO("source: %s", m_source.c_str());
            CALL_MHANDLER_CMD(LOAD_SCENE, msg);
        } else if (property == PROPERTY_ASSETS) {
            msg->findString("value", &m_assets);
            CALL_MHANDLER_CMD(LOAD_SCENE, msg);
        } else if (property == PROPERTY_FPS) {
            int32_t fps { 15 };
            msg->findInt32("value", &fps);
            if (fps >= 5) {
                m_render_handler->frame_timer.SetRequiredFps((uint8_t)fps);
            }
        } else if (property == PROPERTY_FILLMODE) {
            int32_t value;
            if (msg->findInt32("value", &value)) {
                auto nmsg = looper::Message::create(0, m_render_handler);
                addMsgCmd(*nmsg, RenderHandler::CMD::CMD_SET_FILLMODE);
                nmsg->setInt32("value", value);
                nmsg->post();
            }
        } else if (property == PROPERTY_GRAPHIVZ) {
            msg->findBool("value", &m_gen_graphviz);
        } else if (property == PROPERTY_MUTED) {
            bool muted { false };
            msg->findBool("value", &muted);
            m_sound_manager->SetMuted(muted);
        } else if (property == PROPERTY_VOLUME) {
            float volume { 1.0f };
            msg->findFloat("value", &volume);
            m_sound_manager->SetVolume(volume);
        } else if (property == PROPERTY_CACHE_PATH) {
            std::string path;
            msg->findString("value", &path);
            m_cache_path = path;
        }
    }
}

MHANDLER_CMD_IMPL(MainHandler, STOP) {
    bool stop { false };
    if (msg->findBool("value", &stop)) {
        if (stop) {
            m_sound_manager->Pause();
        } else {
            m_sound_manager->Play();
        }

        auto msg_r = looper::Message::create(0, m_render_handler);
        addMsgCmd(*msg_r, RenderHandler::CMD::CMD_STOP);
        msg_r->setBool("value", stop);
        msg_r->post();
    }
}

void MainHandler::loadScene() {
    if (m_source.empty() || m_assets.empty()) return;

    if (! m_sound_manager->IsInited()) {
        m_sound_manager->Init();
        m_sound_manager->Play();
    } else {
        m_sound_manager->UnMountAll();
    }

    std::shared_ptr<Scene> scene { nullptr };

    // mount assets dir
    std::unique_ptr<fs::VFS> pVfs = std::make_unique<fs::VFS>();
    auto&                    vfs  = *pVfs;
    if (! vfs.IsMounted("assets")) {
        bool sus = vfs.Mount("/assets", fs::CreatePhysicalFs(m_assets), "assets");
        if (! sus) {
            LOG_ERROR("Mount assets dir failed");
            return;
        }
    }
    std::filesystem::path pkgPath_fs { m_source };
    pkgPath_fs.replace_extension("pkg");
    std::string pkgPath  = pkgPath_fs.native();
    std::string pkgEntry = pkgPath_fs.filename().replace_extension("json").native();
    std::string pkgDir   = pkgPath_fs.parent_path().native();
    std::string scene_id = pkgPath_fs.parent_path().filename().native();

    // load pkgfile
    if (! vfs.Mount("/assets", fs::WPPkgFs::CreatePkgFs(pkgPath))) {
        LOG_INFO("load pkg file %s failed, fallback to use dir", pkgPath.c_str());
        // load pkg dir
        if (! vfs.Mount("/assets", fs::CreatePhysicalFs(pkgDir))) {
            LOG_ERROR("can't load pkg directory: %s", pkgDir.c_str());
            return;
        }
    }
    if (! m_cache_path.empty()) {
        if (! vfs.Mount("/cache", fs::CreatePhysicalFs(m_cache_path, true), "cache")) {
            LOG_ERROR("can't load cache directory: %s", m_cache_path.c_str());
        }
    }

    {
        std::string       scene_src;
        const std::string base { "/assets/" };
        {
            std::string scenePath = base + pkgEntry;
            if (vfs.Contains(scenePath)) {
                auto f = vfs.Open(scenePath);
                if (f) scene_src = f->ReadAllStr();
            }
        }
        if (scene_src.empty()) {
            LOG_ERROR("Not supported scene type");
            return;
        }
        scene = m_scene_parser.Parse(scene_id, scene_src, vfs, *m_sound_manager);
        scene->vfs.swap(pVfs);
    }

    auto msg = looper::Message::create(0, m_render_handler);
    addMsgCmd(*msg, RenderHandler::CMD::CMD_SET_SCENE);
    msg->setObject("scene", scene);
    msg->post();
}
void MainHandler::sendCmdLoadScene() {
    auto msg = looper::Message::create(0, shared_from_this());
    addMsgCmd(*msg, MainHandler::CMD::CMD_LOAD_SCENE);
    msg->post();
}

bool MainHandler::init() {
    if (m_inited) return true;
    m_main_loop->setName("main");
    m_render_loop->setName("render");

    m_main_loop->start();
    m_render_loop->start();

    m_main_loop->registerHandler(shared_from_this());
    m_render_loop->registerHandler(m_render_handler);

    {
        auto& frameTimer = m_render_handler->frame_timer;
        auto  msg        = looper::Message::create(0, m_render_handler);
        addMsgCmd(*msg, RenderHandler::CMD::CMD_DRAW);

        frameTimer.SetCallback([msg]() {
            msg->post();
        });
        frameTimer.SetRequiredFps(15);
        frameTimer.Run();
    }

    m_inited = true;
    return true;
}
MainHandler::MainHandler()
    : m_sound_manager(std::make_unique<audio::SoundManager>()),
      m_main_loop(std::make_shared<looper::Looper>()),
      m_render_loop(std::make_shared<looper::Looper>()),
      m_render_handler(std::make_shared<RenderHandler>(*this)) {}
