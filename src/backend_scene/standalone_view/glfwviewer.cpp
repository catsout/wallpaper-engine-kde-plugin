#include <iostream>
#include <set>
#include <fstream>

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <atomic>
#include "arg.hpp"
#include "SceneWallpaper.hpp"
#include "SceneWallpaperSurface.hpp"

using namespace std;

atomic<bool> renderCall(false);

struct UserData {
    wallpaper::SceneWallpaper* psw { nullptr };

    uint16_t width;
    uint16_t height;
};

extern "C" {
void framebuffer_size_callback(GLFWwindow*, int width, int height) {}

void mouse_button_callback(GLFWwindow* window, int button, int action, int mods) {
    if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS) {
    }
}

void cursor_position_callback(GLFWwindow* win, double xpos, double ypos) {
    UserData* data = static_cast<UserData*>(glfwGetWindowUserPointer(win));
    data->psw->mouseInput(xpos / data->width, ypos / data->height);
}
}

void updateCallback() {
    renderCall = true;
    glfwPostEmptyEvent();
}

int main(int argc, char** argv) {
    argparse::ArgumentParser program("scene-viewer");
    setAndParseArg(program, argc, argv);
	auto [w_width, w_height] = program.get<Resolution>(OPT_RESOLUTION);

    glfwInit();
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    GLFWwindow* window = glfwCreateWindow(w_width, w_height, "WP", nullptr, nullptr);

    UserData data;
    data.width  = w_width;
    data.height = w_height;

    wallpaper::RenderInitInfo info;
    info.enable_valid_layer = program.get<bool>(OPT_VALID_LAYER);
    info.width              = w_width;
    info.height             = w_height;

    auto& sf_info = info.surface_info;
    {
        uint32_t glfwExtCount = 0;
        auto     exts         = glfwGetRequiredInstanceExtensions(&glfwExtCount);
        for (int i = 0; i < glfwExtCount; i++) {
            sf_info.instanceExts.emplace_back(exts[i]);
        }

        sf_info.createSurfaceOp = [window](VkInstance inst, VkSurfaceKHR* surface) {
            return glfwCreateWindowSurface(inst, window, NULL, surface);
        };
    }

    if (window == nullptr) {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }

    auto* psw = new wallpaper::SceneWallpaper();
    data.psw  = psw;

    psw->init();
    psw->initVulkan(info);
    psw->setPropertyString(wallpaper::PROPERTY_ASSETS, program.get<std::string>(ARG_ASSETS));
    psw->setPropertyString(wallpaper::PROPERTY_SOURCE, program.get<std::string>(ARG_SCENE));
    psw->setPropertyBool(wallpaper::PROPERTY_GRAPHIVZ, program.get<bool>(OPT_GRAPHVIZ));
    psw->setPropertyInt32(wallpaper::PROPERTY_FPS, program.get<int32_t>(OPT_FPS));

    glfwSetWindowUserPointer(window, &data);

    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwSetMouseButtonCallback(window, mouse_button_callback);
    glfwSetCursorPosCallback(window, cursor_position_callback);

    while (! glfwWindowShouldClose(window)) {
        glfwPollEvents();
    }
    delete psw;
    // wgl.Clear();
    glfwDestroyWindow(window);
    glfwTerminate();
    return 0;
}
