include_directories(include/Looper)

set(LIB_NAME wpLooper)

add_library(${LIB_NAME}
STATIC
Looper.cpp
)

target_link_libraries(${LIB_NAME})
target_include_directories(${LIB_NAME} PUBLIC include)
set_property(TARGET ${LIB_NAME} PROPERTY POSITION_INDEPENDENT_CODE ON)
