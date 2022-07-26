set(LIB_NAME wpScene)

add_library(${LIB_NAME}
STATIC

Scene.cpp
SceneCamera.cpp
SceneImageEffectLayer.cpp
SceneIndexArray.cpp
SceneNode.cpp
SceneVertexArray.cpp
SceneShader.cpp
)

target_link_libraries(${LIB_NAME}
PUBLIC
    wpUtils
PRIVATE
    wpFs
    wpParticle
)
target_include_directories(${LIB_NAME} PUBLIC include PRIVATE include/Scene)
target_compile_options(${LIB_NAME} PRIVATE ${warn_opts})
set_property(TARGET ${LIB_NAME} PROPERTY POSITION_INDEPENDENT_CODE ON)
