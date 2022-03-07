project(wallpaper)
set(CMAKE_CXX_STANDARD 17)

find_package(OpenGL REQUIRED)
find_package(Threads REQUIRED)
pkg_check_modules(LZ4 REQUIRED liblz4)

if (CMAKE_BUILD_TYPE STREQUAL "Debug")
	add_compile_definitions(DEBUG_OPENGL=1)
endif ()

include_directories(.)

add_subdirectory(Audio)
add_subdirectory(Vulkan)
add_subdirectory(Looper)
add_subdirectory(RenderGraph)

add_library(${PROJECT_NAME}
	STATIC
	Image.cpp
	WPJson.cpp
	WPPkgFs.cpp
	Type.cpp

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

	Utils/Logging.cpp
	Utils/FrameTimer.cpp
	Utils/FpsCounter.cpp
	Utils/Algorism.cpp	
	Utils/Sha.cpp

	VulkanRender/CopyPass.cpp
	VulkanRender/CustomShaderPass.cpp
	VulkanRender/FinPass.cpp
	VulkanRender/VulkanRender.cpp

	WPParticleParser.cpp
	WPShaderParser.cpp
	WPSceneParser.cpp
	WPShaderValueUpdater.cpp
	WPTexImageParser.cpp
	WPSoundParser.cpp

	SceneWallpaper.cpp	
	SceneToRenderGraph.cpp
)

set(InteralLib wpAudio wpVulkan wpRGraph wpLooper)

set_property(TARGET ${PROJECT_NAME} PROPERTY POSITION_INDEPENDENT_CODE ON)
target_link_libraries(${PROJECT_NAME}
PRIVATE
	${InteralLib}
	OpenGL::GL
	${LZ4_LIBRARIES}
	${CMAKE_THREAD_LIBS_INIT}
	${CMAKE_DL_LIBS}
	glad
)
target_include_directories(${PROJECT_NAME} PUBLIC . Utils Swapchain)
