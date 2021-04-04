project(wallpaper)

find_package(OpenGL REQUIRED)
find_package(Threads REQUIRED)
pkg_check_modules(LZ4 REQUIRED liblz4)

if (CMAKE_BUILD_TYPE STREQUAL "Debug")
	add_compile_definitions(DEBUG_OPENGL=1)
endif ()

add_library(${PROJECT_NAME}
	common.cpp
	common.h
	GLWrapper.cpp
	GLWrapper.h
	GLGraphicManager.h
	GLGraphicManager.cpp
	GraphicManager.h
	pkg.cpp
	pkg.h
	wallpaper.cpp
	wallpaper.h
	Image.h
	Image.cpp
	WPJson.cpp
	WPJson.h
	FpsCounter.h
	FpsCounter.cpp
	FrameTimer.h
	FrameTimer.cpp
	SpriteAnimation.h
	SceneIndexArray.h
	SceneIndexArray.cpp
	SceneVertexArray.h
	SceneVertexArray.cpp
	SceneMesh.h
	Scene.h
	SceneCamera.h
	SceneCamera.cpp
	SceneImageEffect.h
	SceneImageEffectLayer.h
	SceneMaterial.h
	SceneNode.h
	SceneNode.cpp
	SceneTexture.h
	SceneShader.h
	SceneShader.cpp
	Type.h
	Type.cpp
	Interface/ISceneParser.h
	Interface/IShaderValueUpdater.h
	Interface/IImageParser.h
	WPSceneParser.h
	WPSceneParser.cpp
	WPShaderValueUpdater.h
	WPShaderValueUpdater.cpp
	WPTexImageParser.h
	WPTexImageParser.cpp
)
set_property(TARGET ${PROJECT_NAME} PROPERTY POSITION_INDEPENDENT_CODE ON)
target_link_libraries(${PROJECT_NAME}
    PUBLIC 
		OpenGL::GL
		${LZ4_LIBRARIES}
		${CMAKE_THREAD_LIBS_INIT}
		glad
)
target_include_directories(${PROJECT_NAME} PUBLIC .)
