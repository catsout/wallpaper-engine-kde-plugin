#include "SceneWallpaper.hpp"
#include "SceneWallpaperSurface.hpp"

#include "Utils/Logging.h"
#include "Looper/Looper.hpp"

#include "Utils/FrameTimer.h"
#include "WPSceneParser.h"
#include "Scene/Scene.h"

#include "Fs/VFS.h"
#include "Fs/PhysicalFs.h"
#include "WPPkgFs.h"

#include "Audio/SoundManager.h"

#include "RenderGraph/RenderGraph.hpp"
#include "SceneToRenderGraph.hpp"

#include "VulkanRender/VulkanRender.hpp"


using namespace wallpaper;


#define CASE_CMD(cmd) case CMD::CMD_##cmd: handle_##cmd(msg);break;
#define MHANDLER_CMD(cmd) void handle_##cmd(const std::shared_ptr<looper::Message>& msg)

template <typename T>
static void addMsgCmd(looper::Message& msg, T cmd) {
    msg.setInt32("cmd", (int32_t)cmd);
}

namespace wallpaper
{
class MainHandler : public looper::Handler {
public:
    enum class CMD {
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
            CASE_CMD(SET_PROPERTY)
            default:
                break;
            }
        }
        LOG_ERROR("source: %s", m_source.c_str());
    }

    MHANDLER_CMD(SET_PROPERTY) {
        std::string property;
        if(msg->findString("property", &property)) {
            if(property == "source") {
                msg->findString("value", &m_source);
                loadScene();
            }
            if(property == "assets") {
                msg->findString("value", &m_assets);
                loadScene();
            }
        }
    }
    void loadScene();
private:
    std::string m_assets;
    std::string m_source;

    /*
    fs::VFS vfs;
    std::unique_ptr<Scene> scene;
    WPSceneParser parser;
    */
    WPSceneParser m_scene_parser;
};

class RenderHandler : public looper::Handler {
public:
    enum class CMD {
        CMD_INIT_VULKAN,
        CMD_SET_SCENE,
        CMD_NO
    };
    RenderHandler(SceneWallpaper& impl):
        uper(impl),
        m_render(std::make_unique<vulkan::VulkanRender>()) {}
    virtual ~RenderHandler() { frameTimer.Stop(); }
    SceneWallpaper& uper;

    void onMessageReceived(const std::shared_ptr<looper::Message>& msg) override {
        int32_t cmd_int = (int32_t)CMD::CMD_NO;
        if(msg->findInt32("cmd", &cmd_int)) {
            CMD cmd = static_cast<CMD>(cmd_int);
            switch(cmd) {
            CASE_CMD(SET_SCENE)
            CASE_CMD(INIT_VULKAN)
            }
        }
        else if(!frameTimer.NoFrame()) {
            LOG_INFO("render one");
            frameTimer.RenderFrame();
        }
    }

    MHANDLER_CMD(SET_SCENE) {
        if(msg->findObject("scene", &m_scene)) {
            LOG_ERROR("-----------");
            m_rg = sceneToRenderGraph(*m_scene);
            m_rg->ToGraphviz("graph.dot");
        }
    }
    MHANDLER_CMD(INIT_VULKAN) {
        LOG_INFO("running with surface");
        std::shared_ptr<VulkanSurfaceInfo> info;
        msg->findObject("info", &info);
        m_render->init(*info);
    }
public:
	FrameTimer frameTimer;
private:
    std::shared_ptr<Scene> m_scene;
    std::unique_ptr<rg::RenderGraph> m_rg;
    std::unique_ptr<vulkan::VulkanRender> m_render;
};
}

class SceneWallpaper::impl {
public:
    impl(SceneWallpaper& uper):
        uper(uper),
        main_loop(std::make_shared<looper::Looper>()),
        render_loop(std::make_shared<looper::Looper>()),
        main_handler(std::make_shared<MainHandler>(uper)),
        render_handler(std::make_shared<RenderHandler>(uper)) {};

    ~impl()	{}
    SceneWallpaper& uper;
    std::shared_ptr<looper::Looper> main_loop;
    std::shared_ptr<looper::Looper> render_loop;
    std::shared_ptr<MainHandler> main_handler;
    std::shared_ptr<RenderHandler> render_handler;
};

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
        auto& frameTimer = pImpl->render_handler->frameTimer;
        auto msg = looper::Message::create(0, pImpl->render_handler);
        frameTimer.SetCallback([msg]() {
            msg->post();
        });
        frameTimer.SetFps(10);
        frameTimer.Run();
    }

    m_inited = true;
    return true;
}

void SceneWallpaper::initVulkan(const std::shared_ptr<VulkanSurfaceInfo>& info) {
    auto msg = looper::Message::create(0, pImpl->render_handler);
    addMsgCmd(*msg, RenderHandler::CMD::CMD_INIT_VULKAN);
    msg->setObject("info", info);
    msg->post();
}

void SceneWallpaper::initVulkanEx(Span<uint8_t> uuid) {
    m_exswap_mode = true;
}

void SceneWallpaper::draw() {
}

void SceneWallpaper::setProperty(std::string_view name, std::string_view value) {
    auto msg = looper::Message::create(0, pImpl->main_handler);
    addMsgCmd(*msg, MainHandler::CMD::CMD_SET_PROPERTY);
    msg->setString("property", std::string(name));
    msg->setString("value", std::string(value));
    msg->post();
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

