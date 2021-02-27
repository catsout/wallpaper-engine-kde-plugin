#include <iostream>
#include <set>
#include <fstream>
#include <map>
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include "pkg.h"
#include "wallpaper.h"

using namespace std;

const unsigned int SCR_WIDTH = 1280;
const unsigned int SCR_HEIGHT = 720;

int main(int argc, char**argv)
{
	if(argc != 3){
		std::cerr << "usage: " << argv[0] << " <assets dir> <pkg file>" << std::endl;
		return 1;
	}
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "WP", NULL, NULL);
    if (window == NULL)
    {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);
    //glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

    wallpaper::WallpaperGL* wgl_ptr = new wallpaper::WallpaperGL();
    auto& wgl = *wgl_ptr;
	wgl.Init((GLADloadproc)glfwGetProcAddress);
	wgl.SetAssets(argv[1]);
    //const wallpaper::fs::file_node& x = wallpaper::WallpaperGL::GetPkgfs();
    wgl.Load(argv[2]);

    while (!glfwWindowShouldClose(window))
    {
		glfwPollEvents();
        wgl.Render(0,SCR_WIDTH,SCR_HEIGHT);
        glfwSwapBuffers(window);
    }
    delete wgl_ptr;
    //wgl.Clear();
    glfwTerminate();
    return 0;
}
