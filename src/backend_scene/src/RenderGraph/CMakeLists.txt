set(LIB_NAME wpRGraph)
add_library(${LIB_NAME}
STATIC
DependencyGraph.cpp
TexNode.cpp
PassNode.cpp
RenderGraph.cpp
)

target_link_libraries(${LIB_NAME} PUBLIC wpUtils)
target_include_directories(${LIB_NAME} PUBLIC include PRIVATE include/RenderGraph)
set_property(TARGET ${LIB_NAME} PROPERTY POSITION_INDEPENDENT_CODE ON)
