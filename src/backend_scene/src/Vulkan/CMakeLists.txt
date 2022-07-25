set(LIB_NAME wpVulkan)

find_package(Vulkan REQUIRED)

add_library(${LIB_NAME}
STATIC
Instance.cpp
Device.cpp
GraphicsPipeline.cpp
Shader.cpp
StagingBuffer.cpp
Swapchain.cpp
TextureCache.cpp
Parameters.cpp
Vma.cpp
vulkan_wrapper.cpp
)

target_link_libraries(${LIB_NAME}
PUBLIC
	glslang
    spirv-reflect
	${Vulkan_LIBRARIES}
    wpScene
PRIVATE
	SPIRV
)
target_include_directories(${LIB_NAME} PUBLIC include PRIVATE include/Vulkan include/vkk)
target_compile_options(${LIB_NAME} PRIVATE ${warn_opts} -Wno-missing-field-initializers)
set_property(TARGET ${LIB_NAME} PROPERTY POSITION_INDEPENDENT_CODE ON)
