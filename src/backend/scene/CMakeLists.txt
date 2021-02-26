set(CMAKE_CXX_STANDARD 14)
project(scenebackend)

if (CMAKE_BUILD_TYPE EQUAL "DEBUG")
add_compile_definitions(DEBUG_OPENGL=1)
endif (CMAKE_BUILD_TYPE EQUAL "DEBUG")

find_package(Qt5 COMPONENTS Gui Quick Qml X11Extras REQUIRED)
include_directories(${Qt5Gui_PRIVATE_INCLUDE_DIRS})
include_directories(include)

add_library(wallpaper
	src/common.cpp
	src/common.h
	src/effect.cpp
	src/effect.h
	src/GLVertice.cpp
	src/GLVertice.h
	src/GLWrapper.cpp
	src/GLWrapper.h
	src/image.cpp
	src/image.h
	src/material.cpp
	src/material.h
	src/object.cpp
	src/object.h
	src/pkg.cpp
	src/pkg.h
	src/teximage.cpp
	src/teximage.h
	src/wallpaper.cpp
	src/wallpaper.h
	src/WPRender.cpp
	src/WPRender.h
	src/WPShaderManager.cpp
	src/WPShaderManager.h
	src/WPTextureCache.cpp
	src/WPTextureCache.h
)
add_library(GLAD "thirdparty/glad.c")
set_property(TARGET wallpaper GLAD PROPERTY POSITION_INDEPENDENT_CODE ON)
target_link_libraries(wallpaper
    PUBLIC 
		GLAD
		GL
		dl
		lz4
)
target_include_directories(wallpaper PUBLIC src include)

add_library(${PROJECT_NAME}
	SceneViewer.cpp
	SceneViewer.h
)
target_link_libraries(${PROJECT_NAME}
    PUBLIC 
        Qt5::Quick
        Qt5::Gui
		Qt5::X11Extras
		wallpaper
)
target_include_directories(${PROJECT_NAME} PUBLIC .)


add_executable(${PROJECT_NAME}-viewer-qml
	viewer/qmlviewer.cpp
	viewer/main.qml
	viewer/pkg.qrc
)
target_link_libraries(${PROJECT_NAME}-viewer-qml
    PUBLIC 
        Qt5::Quick
        Qt5::Gui
		${PROJECT_NAME}
)

add_executable(${PROJECT_NAME}-viewer
	viewer/glfwviewer.cpp
)
target_link_libraries(${PROJECT_NAME}-viewer
    PUBLIC 
		glfw
		${PROJECT_NAME}
)
