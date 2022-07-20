
find_package(OpenGL REQUIRED)
find_package(Threads REQUIRED)
pkg_check_modules(LZ4 REQUIRED liblz4)

option(ENABLE_RENDERDOC "Build with renderdoc api" OFF)

if (ENABLE_RENDERDOC)
	add_compile_definitions(ENABLE_RENDERDOC_API=1)
endif()

include(TestBigEndian)
TEST_BIG_ENDIAN(ENDIAN)
if (ENDIAN)
	add_compile_definitions(WP_BIG_ENDIAN)
endif ()


include_directories(.)

add_subdirectory(Audio)
add_subdirectory(Looper)
add_subdirectory(Scene)
add_subdirectory(Vulkan)
add_subdirectory(RenderGraph)

add_library(${PROJECT_NAME}
	STATIC
	WPJson.cpp
	WPPkgFs.cpp
	Type.cpp

	wpscene/WPImageObject.cpp
	wpscene/WPParticleObject.cpp
	wpscene/WPMaterial.cpp
	wpscene/WPScene.cpp
	wpscene/WPLightObject.cpp
	Particle/ParticleModify.cpp
	Particle/ParticleSystem.cpp
	Particle/ParticleEmitter.cpp
	Particle/WPParticleRawGener.cpp

	Utils/Logging.cpp
	Utils/FrameTimer.cpp
	Utils/FpsCounter.cpp
	Utils/Algorism.cpp	
	Utils/Sha.cpp
    Utils/DynamicLibrary.cpp

    VulkanRender/CopyPass.cpp
    VulkanRender/CustomShaderPass.cpp
    VulkanRender/FinPass.cpp
    VulkanRender/PrePass.cpp
    VulkanRender/SceneToRenderGraph.cpp
	VulkanRender/VulkanRender.cpp

	WPParticleParser.cpp
	WPShaderParser.cpp
	WPSceneParser.cpp
	WPShaderValueUpdater.cpp
	WPTexImageParser.cpp
	WPSoundParser.cpp
	WPMdlParser.cpp
	WPPuppet.cpp

	SceneWallpaper.cpp	
)

set(InteralLib wpAudio wpVulkan wpRGraph wpLooper wpScene)

set_property(TARGET ${PROJECT_NAME} PROPERTY POSITION_INDEPENDENT_CODE ON)
target_link_libraries(${PROJECT_NAME}
PRIVATE
	${InteralLib}
	${LZ4_LIBRARIES}
	${CMAKE_THREAD_LIBS_INIT}
	${CMAKE_DL_LIBS}
)
target_include_directories(${PROJECT_NAME} PUBLIC . Swapchain)
