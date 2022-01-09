add_library(wpAudio
    STATIC
    SoundManager.cpp
)

target_link_libraries(wpAudio)
target_include_directories(wpAudio PUBLIC .)
set_property(TARGET wpAudio PROPERTY POSITION_INDEPENDENT_CODE ON)
