include_directories(include/Vulkan)

set(LIB_NAME wpVulkan)

add_library(${LIB_NAME}
STATIC
Instance.cpp
Vma.cpp
)

target_link_libraries(${LIB_NAME}
PUBLIC
	glslang
PRIVATE
	SPIRV
	vulkan
	pthread
	dl
)
target_include_directories(${LIB_NAME} PUBLIC include)
set_property(TARGET ${LIB_NAME} PROPERTY POSITION_INDEPENDENT_CODE ON)
