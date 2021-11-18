project(glad)

add_library(${PROJECT_NAME} STATIC include/glad/glad.h src/glad.c)
set_property(TARGET ${PROJECT_NAME} PROPERTY POSITION_INDEPENDENT_CODE ON)
target_link_libraries(${PROJECT_NAME} PRIVATE ${CMAKE_DL_LIBS})
target_include_directories(${PROJECT_NAME} PUBLIC include)
