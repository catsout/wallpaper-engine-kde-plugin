project(spirv-reflect)

add_compile_definitions(SPIRV_REFLECT_ENABLE_ASSERTS=1)

add_library(${PROJECT_NAME} STATIC ${CMAKE_CURRENT_SOURCE_DIR}/spirv_reflect.h
                                     ${CMAKE_CURRENT_SOURCE_DIR}/spirv_reflect.c)

target_include_directories(${PROJECT_NAME} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
