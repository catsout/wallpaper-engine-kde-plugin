set(LIB_NAME wpParticle)

add_library(${LIB_NAME}
STATIC
ParticleModify.cpp
ParticleSystem.cpp
ParticleEmitter.cpp
WPParticleRawGener.cpp
)

target_link_libraries(${LIB_NAME} PUBLIC wpUtils PRIVATE wpScene)
target_include_directories(${LIB_NAME} PUBLIC include PRIVATE include/Particle)
set_property(TARGET ${LIB_NAME} PROPERTY POSITION_INDEPENDENT_CODE ON)
