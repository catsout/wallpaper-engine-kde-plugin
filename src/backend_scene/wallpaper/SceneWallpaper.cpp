#include "SceneWallpaper.hpp"
#include "SceneWallpaperSurface.hpp"

#include "Utils/Logging.h"
#include "Looper/Looper.hpp"

#include "Utils/FrameTimer.h"
#include "Utils/FpsCounter.h"
#include "WPSceneParser.h"
#include "Scene/Scene.h"

#include "Fs/VFS.h"
#include "Fs/PhysicalFs.h"
#include "WPPkgFs.h"

#include "Audio/SoundManager.h"

#include "RenderGraph/RenderGraph.hpp"
#include "SceneToRenderGraph.hpp"

#include "VulkanRender/VulkanRender.hpp"
#include "Vulkan/VulkanExSwapchain.hpp"
#include <atomic>


using namespace wallpaper;


#define CASE_CMD(cmd) case CMD::CMD_##cmd: handle_##cmd(msg);break;
#define MHANDLER_CMD(cmd) void handle_##cmd(const std::shared_ptr<looper::Message>& msg)
#define MHANDLER_CMD_IMPL(cl, cmd) void impl_##cl::handle_##cmd(const std::shared_ptr<looper::Message>& msg)
#define CALL_MHANDLER_CMD(cmd, msg) handle_##cmd(msg)

template <typename T>
static void addMsgCmd(looper::Message& msg, T cmd) {
    msg.setInt32("cmd", (int32_t)cmd);
}

class SceneWallpaper::impl {
public:
    impl(SceneWallpaper& uper);

    ~impl()	{}
    SceneWallpaper& uper;
    std::shared_ptr<looper::Looper> main_loop;
    std::shared_ptr<looper::Looper> render_loop;
    std::shared_ptr<MainHandler> main_handler;
    std::shared_ptr<RenderHandler> render_handler;
};

namespace wallpaper
{
class MainHandler : public looper::Handler {
public:
    enum class CMD {
        CMD_LOAD_SCENE,
        CMD_SET_PROPERTY,
        CMD_NO
    };
public:
    MainHandler(SceneWallpaper& impl):uper(impl) {}
    virtual ~MainHandler() = default;
    SceneWallpaper& uper;
public:
    void onMessageReceived(const std::shared_ptr<looper::Message>& msg) override {
        int32_t cmd_int = (int32_t)CMD::CMD_NO;
        if(msg->findInt32("cmd", &cmd_int)) {
            CMD cmd = static_cast<CMD>(cmd_int);
            switch (cmd)
            {
            CASE_CMD(SET_PROPERTY);
            CASE_CMD(LOAD_SCENE);
            default:
                break;
            }
        }
    }

    void sendCmdLoadScene();
    bool isGenGraphviz() const { return m_gen_graphviz; }

private:
    void loadScene();

    MHANDLER_CMD(LOAD_SCENE);
    MHANDLER_CMD(SET_PROPERTY);

private:
    std::string m_assets;
    std::string m_source;
    bool m_gen_graphviz {false};

    /*
    fs::VFS vfs;
    std::unique_ptr<Scene> scene;
    WPSceneParser parser;
    */
    WPSceneParser m_scene_parser;
};
// for macro
using impl_MainHandler = MainHandler;

class RenderHandler : public looper::Handler {
public:
    enum class CMD {
        CMD_INIT_VULKAN,
        CMD_SET_SCENE,
        CMD_SET_FILLMODE,
        CMD_STOP,
        CMD_DRAW,
        CMD_NO
    };
    RenderHandler(SceneWallpaper& impl):
        uper(impl),
        m_render(std::make_unique<vulkan::VulkanRender>()) {}
    virtual ~RenderHandler() { 
        frame_timer.Stop(); 
        m_render->destroy();
    }
 

    void onMessageReceived(const std::shared_ptr<looper::Message>& msg) override {
        int32_t cmd_int = (int32_t)CMD::CMD_NO;
        if(msg->findInt32("cmd", &cmd_int)) {
            CMD cmd = static_cast<CMD>(cmd_int);
            switch(cmd) {
            CASE_CMD(DRAW)
            CASE_CMD(STOP)
            CASE_CMD(SET_FILLMODE)
            CASE_CMD(SET_SCENE)
            CASE_CMD(INIT_VULKAN)
            }
        }
    }

    vulkan::VulkanExSwapchain* exSwapchain() const {
        return m_render->exSwapchain();
    }

    bool renderInited() const {
        return m_render->inited();
    }

private:
    MHANDLER_CMD(STOP) {
        bool stop {false};
        if(msg->findBool("value", &stop)) {
            if(stop) frame_timer.Stop();
            else frame_timer.Run();
        }
    }
    MHANDLER_CMD(DRAW) {
        frame_timer.FrameBegin();
        if(m_rg) {
            //LOG_INFO("frame info, fps: %.1f, frametime: %.1f", 1.0f, 1000.0f*m_scene->frameTime);
            m_scene->shaderValueUpdater->FrameBegin();
            m_scene->paritileSys.Emitt();

            m_render->drawFrame(*m_scene);

            m_scene->PassFrameTime(frame_timer.IdeaTime());

            m_scene->shaderValueUpdater->FrameEnd();
            //fps_counter.RegisterFrame();
        }
        frame_timer.FrameEnd();
    }
    MHANDLER_CMD(SET_FILLMODE) {
        int32_t value;
        if(msg->findInt32("value", &value)) {
            m_fillmode = (FillMode)value;
            if(m_scene && renderInited()) {
                m_render->UpdateCameraFillMode(*m_scene, m_fillmode);
            }
        }
    }
    MHANDLER_CMD(SET_SCENE) {
        if(msg->findObject("scene", &m_scene)) {
            decltype(m_rg) old_rg = std::move(m_rg);
            m_rg = sceneToRenderGraph(*m_scene);
            if(uper.pImpl->main_handler->isGenGraphviz())
                m_rg->ToGraphviz("graph.dot");
            m_render->compileRenderGraph(*m_scene, *m_rg);
            m_render->UpdateCameraFillMode(*m_scene, m_fillmode);
        }
    }
    MHANDLER_CMD(INIT_VULKAN) {
        std::shared_ptr<RenderInitInfo> info;
        if(msg->findObject("info", &info)) {
            m_render->init(*info);

            // inited, callback to laod scene
            uper.pImpl->main_handler->sendCmdLoadScene();
        }
    }


public:
	FrameTimer frame_timer;
	FpsCounter fps_counter;;
private:
    SceneWallpaper& uper;
    std::shared_ptr<Scene> m_scene {nullptr};
    std::unique_ptr<rg::RenderGraph> m_rg {nullptr};
    std::unique_ptr<vulkan::VulkanRender> m_render;
    FillMode m_fillmode {FillMode::ASPECTCROP};
};
}



SceneWallpaper::SceneWallpaper():pImpl(std::make_unique<impl>(*this)) {}

SceneWallpaper::~SceneWallpaper() {}

bool SceneWallpaper::inited() const {
    return m_inited;
}

bool SceneWallpaper::init() {
    if(m_inited) return true;
    auto& main_loop = pImpl->main_loop;
    auto& render_loop = pImpl->render_loop;

    main_loop->setName("main");
    render_loop->setName("render");

    main_loop->start();
    render_loop->start();

    main_loop->registerHandler(pImpl->main_handler);
    render_loop->registerHandler(pImpl->render_handler);

    {
        auto& frameTimer = pImpl->render_handler->frame_timer;
        auto msg = looper::Message::create(0, pImpl->render_handler);
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

void SceneWallpaper::initVulkan(const RenderInitInfo& info) {
    std::shared_ptr<RenderInitInfo> sp_info = std::make_shared<RenderInitInfo>(info);
    auto msg = looper::Message::create(0, pImpl->render_handler);
    addMsgCmd(*msg, RenderHandler::CMD::CMD_INIT_VULKAN);
    msg->setObject("info", sp_info);
    msg->post();
}

void SceneWallpaper::initVulkanEx(Span<uint8_t> uuid) {
    m_exswap_mode = true;
}

void SceneWallpaper::play() {
    auto msg = looper::Message::create(0, pImpl->render_handler);
    addMsgCmd(*msg, RenderHandler::CMD::CMD_STOP);
    msg->setBool("value", false);
    msg->post();
}
void SceneWallpaper::pause() {
    auto msg = looper::Message::create(0, pImpl->render_handler);
    addMsgCmd(*msg, RenderHandler::CMD::CMD_STOP);
    msg->setBool("value", true);
    msg->post();
}

#define BASIC_TYPE(NAME,TYPENAME)                                        \
void SceneWallpaper::setProperty##NAME(std::string_view name, TYPENAME value) {    \
    auto msg = looper::Message::create(0, pImpl->main_handler);          \
    addMsgCmd(*msg, MainHandler::CMD::CMD_SET_PROPERTY);                 \
    msg->setString("property", std::string(name));                       \
    msg->set##NAME("value", value);                                      \
    msg->post();                                                         \
}

BASIC_TYPE(Bool, bool);
BASIC_TYPE(Int32, int32_t);
BASIC_TYPE(Float, float);
BASIC_TYPE(String, std::string);

ExSwapchain* SceneWallpaper::exSwapchain() const {
    return pImpl->render_handler->exSwapchain();
}

MHANDLER_CMD_IMPL(MainHandler, LOAD_SCENE) {
    if(uper.pImpl->render_handler->renderInited()) {
        loadScene();
    }
}

MHANDLER_CMD_IMPL(MainHandler, SET_PROPERTY) {
    std::string property;
    if(msg->findString("property", &property)) {
        if(property == PROPERTY_SOURCE) {
            msg->findString("value", &m_source);
            LOG_INFO("source: %s", m_source.c_str());
            CALL_MHANDLER_CMD(LOAD_SCENE, msg);
        } else if(property == PROPERTY_ASSETS) {
            msg->findString("value", &m_assets);
            CALL_MHANDLER_CMD(LOAD_SCENE, msg);
        } else if(property == PROPERTY_FPS) {
            int32_t fps {15};
            msg->findInt32("value", &fps);
            if(fps >= 5) {
                uper.pImpl->render_handler->frame_timer.SetRequiredFps((uint8_t)fps);
            }
        } else if(property == PROPERTY_FILLMODE) {
            int32_t value;
            if(msg->findInt32("value", &value)) {
                auto nmsg = looper::Message::create(0, uper.pImpl->render_handler);
                addMsgCmd(*nmsg, RenderHandler::CMD::CMD_SET_FILLMODE);
                nmsg->setInt32("value", value);
                nmsg->post();
            }
        } else if(property == PROPERTY_GRAPHIVZ) {
            msg->findBool("value", &m_gen_graphviz);
        }
    }
}


void MainHandler::loadScene() {
    if(m_source.empty() || m_assets.empty()) 
        return;

    std::shared_ptr<Scene> scene {nullptr};

    //mount assets dir
    std::unique_ptr<fs::VFS> pVfs = std::make_unique<fs::VFS>();
    auto& vfs = *pVfs;
    if(!vfs.IsMounted("assets")) {
        bool sus = vfs.Mount("/assets",
            fs::CreatePhysicalFs(m_assets),
            "assets"
        );
        if(!sus) { 
            LOG_ERROR("Mount assets dir failed");
            return;
        }
    }
    std::filesystem::path pkgPath_fs {m_source};
    pkgPath_fs.replace_extension("pkg");
    std::string pkgPath = pkgPath_fs.native();
    std::string pkgEntry = pkgPath_fs.filename().replace_extension("json").native();
    std::string pkgDir = pkgPath_fs.parent_path().native();
    //load pkgfile
    if(!vfs.Mount("/assets", fs::WPPkgFs::CreatePkgFs(pkgPath))) {
        LOG_INFO("load pkg file %s failed, fallback to use dir", pkgPath.c_str());
        //load pkg dir
        if(!vfs.Mount("/assets", fs::CreatePhysicalFs(pkgDir))) {
            LOG_ERROR("Can't load pkg directory: %s", pkgDir.c_str());
            return;
        }
    }

    {
        std::string scene_src;
        const std::string base {"/assets/"};
        {
            std::string scenePath = base + pkgEntry;
            if(vfs.Contains(scenePath)) {
                auto f = vfs.Open(scenePath);
                if(f) scene_src = f->ReadAllStr();
            }
        }
        if(scene_src.empty()) {
            LOG_ERROR("Not supported scene type");
            return;
        }
        audio::SoundManager sm;
        scene = m_scene_parser.Parse(scene_src, vfs, sm);	
        scene->vfs.swap(pVfs);
    }

    auto msg = looper::Message::create(0, uper.pImpl->render_handler);
    addMsgCmd(*msg, RenderHandler::CMD::CMD_SET_SCENE);
    msg->setObject("scene", scene);
    msg->post();
}
void MainHandler::sendCmdLoadScene() {
    auto msg = looper::Message::create(0, shared_from_this());
    addMsgCmd(*msg, MainHandler::CMD::CMD_LOAD_SCENE);
    msg->post();
}

SceneWallpaper::impl::impl(SceneWallpaper& uper):
        uper(uper),
        main_loop(std::make_shared<looper::Looper>()),
        render_loop(std::make_shared<looper::Looper>()),
        main_handler(std::make_shared<MainHandler>(uper)),
        render_handler(std::make_shared<RenderHandler>(uper)) {};
