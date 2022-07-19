include_directories(include/Vulkan)
include_directories(include/vkk)

set(LIB_NAME wpVulkan)

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
PRIVATE
	SPIRV
	vulkan
	pthread
	dl
)
target_include_directories(${LIB_NAME} PUBLIC include)
set_property(TARGET ${LIB_NAME} PROPERTY POSITION_INDEPENDENT_CODE ON)
