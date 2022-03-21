include_directories(include/RenderGraph)

add_library(wpRGraph
STATIC
DependencyGraph.cpp
TexNode.cpp
PassNode.cpp
RenderGraph.cpp
)

target_link_libraries(wpRGraph
PRIVATE
)
target_include_directories(wpRGraph PUBLIC include)
set_property(TARGET wpRGraph PROPERTY POSITION_INDEPENDENT_CODE ON)
