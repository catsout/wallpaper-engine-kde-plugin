project(wallpaper)

find_package(OpenGL REQUIRED)
find_package(Threads REQUIRED)
pkg_check_modules(LZ4 REQUIRED liblz4)

if (CMAKE_BUILD_TYPE STREQUAL "Debug")
	add_compile_definitions(DEBUG_OPENGL=1)
endif ()

add_library(${PROJECT_NAME}
	Util.cpp
	Log.cpp
	GLWrapper.cpp
	GLGraphicManager.cpp
	pkg.cpp
	wallpaper.cpp
	Image.cpp
	WPJson.cpp
	FpsCounter.cpp
	FrameTimer.cpp
	Scene.cpp
	SceneIndexArray.cpp
	SceneVertexArray.cpp
	SceneCamera.cpp
	SceneNode.cpp
	SceneShader.cpp
	Type.cpp
	wpscene/WPImageObject.cpp
	wpscene/WPParticleObject.cpp
	wpscene/WPMaterial.cpp
	wpscene/WPScene.cpp
	Particle/ParticleSystem.cpp
	Particle/ParticleEmitter.cpp
	Particle/ParticleInitializer.cpp
	Particle/WPParticleRawGener.cpp
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
