#include <iostream>
#include <set>
#include <fstream>

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <atomic>
#include "SceneWallpaper.hpp"
#include "SceneWallpaperSurface.hpp"

using namespace std;

unsigned int SCR_WIDTH = 1280;
unsigned int SCR_HEIGHT = 720;
atomic<bool> renderCall(false);
wallpaper::SceneWallpaper* psw = nullptr;

void framebuffer_size_callback(GLFWwindow*, int width, int height) {
	SCR_WIDTH = width;
	SCR_HEIGHT = height;
	/*
	if(pwgl != nullptr) {
		(*pwgl).SetDefaultFbo(0,SCR_WIDTH,SCR_HEIGHT);
	}
	*/
}

void mouse_button_callback(GLFWwindow* window, int button, int action, int mods) {
    if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS) {
	}
}


void cursor_position_callback(GLFWwindow*, double xpos, double ypos) {
	//if(pwgl) pwgl->SetMousePos(xpos, ypos);
}

void updateCallback() {
	renderCall = true;
	glfwPostEmptyEvent();
}

int main(int argc, char**argv)
{
	if(argc != 3) {
		std::cerr << "usage: "+ std::string(argv[0]) +" <assets dir> <pkg file>\n";
		return 1;
	}

	glfwInit();
	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "WP", nullptr, nullptr);

	auto sf_info = std::make_shared<wallpaper::VulkanSurfaceInfo>();
	{
		uint32_t glfwExtCount = 0;
		auto exts = glfwGetRequiredInstanceExtensions(&glfwExtCount);
		for(int i=0;i < glfwExtCount;i++) {
			sf_info->instanceExts.emplace_back(exts[i]);
		}

		sf_info->createSurfaceOp = [window](VkInstance inst, VkSurfaceKHR* surface) {
			return glfwCreateWindowSurface(inst, window, NULL, surface);
		};
	}

    if (window == nullptr) {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
	glfwSetMouseButtonCallback(window, mouse_button_callback);

 	// glfw callback
	glfwSetCursorPosCallback(window, cursor_position_callback);

    psw = new wallpaper::SceneWallpaper();
	psw->init();
	psw->initVulkan(sf_info);

    while (!glfwWindowShouldClose(window)) {
		glfwPollEvents();
    }
    delete psw;
    //wgl.Clear();
	glfwDestroyWindow(window);
    glfwTerminate();
    return 0;
}
