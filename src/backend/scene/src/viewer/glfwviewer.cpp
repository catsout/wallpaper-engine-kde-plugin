#include <iostream>
#include <set>
#include <fstream>
#include <map>
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include "pkg.h"
#include "wallpaper.h"

#include <unistd.h>

using namespace std;

unsigned int SCR_WIDTH = 1280;
unsigned int SCR_HEIGHT = 720;

void framebuffer_size_callback(GLFWwindow* window, int width, int height) {
	SCR_WIDTH = width;
	SCR_HEIGHT = height;
}

int main(int argc, char**argv)
{
	int result = 0,objnum = -1,effnum = -1;
    while((result = getopt(argc, argv, "o:e:")) != -1 ) {
		switch(result)
		{
		case 'o':
			objnum = std::stoi(optarg);		
			break;
		case 'e':
			effnum = std::stoi(optarg);
			break;
			break;
		case '?':
			break;
		}
    }
	if(argc - optind != 2) {
		std::cerr << "usage: "+ std::string(argv[0]) +" [options] <assets dir> <pkg file>\n"
		<< "options:\n"  
		<< "-o object amount to render\n" 
		<< "-e effect amount to render\n";
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
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

    wallpaper::WallpaperGL* wgl_ptr = new wallpaper::WallpaperGL();
    auto& wgl = *wgl_ptr;
	wgl.Init((GLADloadproc)glfwGetProcAddress);
	wgl.SetAssets(argv[optind]);
	wgl.SetObjEffNum(objnum, effnum);
	wgl.SetFlip(true);
    //const wallpaper::fs::file_node& x = wallpaper::WallpaperGL::GetPkgfs();
    wgl.Load(argv[optind+1]);


	glClear(GL_COLOR_BUFFER_BIT);
    while (!glfwWindowShouldClose(window))
    {
		glfwPollEvents();
		double xpos, ypos;
		glfwGetCursorPos(window, &xpos, &ypos);
		wgl.SetMousePos(xpos, ypos);
        wgl.Render(0,SCR_WIDTH,SCR_HEIGHT);
        glfwSwapInterval(1.5);
        glfwSwapBuffers(window);
    }
    delete wgl_ptr;
    //wgl.Clear();
    glfwTerminate();
    return 0;
}
