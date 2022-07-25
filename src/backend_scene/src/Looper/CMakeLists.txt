set(LIB_NAME wpLooper)

add_library(${LIB_NAME}
STATIC
Looper.cpp
)

target_link_libraries(${LIB_NAME} PUBLIC wpUtils)
target_include_directories(${LIB_NAME} PUBLIC include PRIVATE include/Looper)
target_compile_options(${LIB_NAME} PRIVATE ${warn_opts})
set_property(TARGET ${LIB_NAME} PROPERTY POSITION_INDEPENDENT_CODE ON)
