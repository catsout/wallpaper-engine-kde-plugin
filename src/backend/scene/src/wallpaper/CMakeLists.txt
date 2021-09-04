project(wallpaper)
set(CMAKE_CXX_STANDARD 17)

find_package(OpenGL REQUIRED)
find_package(Threads REQUIRED)
pkg_check_modules(LZ4 REQUIRED liblz4)

if (CMAKE_BUILD_TYPE STREQUAL "Debug")
	add_compile_definitions(DEBUG_OPENGL=1)
endif ()

add_library(${PROJECT_NAME}
	Util.cpp
	wallpaper.cpp
	Image.cpp
	WPJson.cpp
	WPPkgFs.cpp
	Type.cpp
	Utils/Logging.cpp
	Utils/FpsCounter.cpp
	Utils/FrameTimer.cpp
	Utils/Algorism.cpp
	wpscene/WPImageObject.cpp
	wpscene/WPParticleObject.cpp
	wpscene/WPMaterial.cpp
	wpscene/WPScene.cpp
	Particle/ParticleModify.cpp
	Particle/ParticleSystem.cpp
	Particle/ParticleEmitter.cpp
	Particle/WPParticleRawGener.cpp
	Scene/Scene.cpp
	Scene/SceneIndexArray.cpp
	Scene/SceneVertexArray.cpp
	Scene/SceneCamera.cpp
	Scene/SceneNode.cpp
	Scene/SceneShader.cpp
	Renderer/GLWrapper.cpp
	Renderer/GLGraphicManager.cpp
	Renderer/Fg/DependencyGraph.cpp
	Renderer/Fg/FrameGraph.cpp

	WPParticleParser.cpp
	WPShaderParser.cpp
	WPSceneParser.cpp
	WPShaderValueUpdater.cpp
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
