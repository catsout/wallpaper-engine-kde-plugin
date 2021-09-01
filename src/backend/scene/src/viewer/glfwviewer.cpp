#include <iostream>
#include <set>
#include <fstream>
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <atomic>
#include "pkg.h"
#include "wallpaper.h"

using namespace std;

unsigned int SCR_WIDTH = 1280;
unsigned int SCR_HEIGHT = 720;
atomic<bool> renderCall(false);
wallpaper::WallpaperGL* pwgl = nullptr;

void framebuffer_size_callback(GLFWwindow*, int width, int height) {
	SCR_WIDTH = width;
	SCR_HEIGHT = height;
	if(pwgl != nullptr) {
		(*pwgl).SetDefaultFbo(0,SCR_WIDTH,SCR_HEIGHT);
	}
}

void cursor_position_callback(GLFWwindow*, double xpos, double ypos) {
	if(pwgl)
		pwgl->SetMousePos(xpos, ypos);
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
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "WP", nullptr, nullptr);
    if (window == nullptr) {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

    pwgl = new wallpaper::WallpaperGL();
    auto& wgl = *pwgl;
	wgl.Init((GLADloadproc)glfwGetProcAddress);
	wgl.SetAssets(argv[1]);
	wgl.SetFlip(true);
    //const wallpaper::fs::file_node& x = wallpaper::WallpaperGL::GetPkgfs();
    wgl.Load(argv[2]);
	wgl.OutGraphviz("framegraph.dot");

	wgl.SetUpdateCallback(updateCallback);

	// glfw callback
	glfwSetCursorPosCallback(window, cursor_position_callback);

	wgl.SetDefaultFbo(0,SCR_WIDTH,SCR_HEIGHT);

    while (!glfwWindowShouldClose(window)) {
		glfwWaitEvents();
		if(renderCall) {
			renderCall = false;
			wgl.Render();
			glfwSwapBuffers(window);
		}
    }
    delete pwgl;
    //wgl.Clear();
    glfwTerminate();
    return 0;
}
