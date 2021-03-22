project(wallpaper)

find_package(OpenGL REQUIRED)
find_package(Threads REQUIRED)
pkg_check_modules(LZ4 REQUIRED liblz4)

if (CMAKE_BUILD_TYPE STREQUAL "Debug")
	add_compile_definitions(DEBUG_OPENGL=1)
endif ()

add_library(${PROJECT_NAME}
	common.cpp
	common.h
	effect.cpp
	effect.h
	GLVertice.cpp
	GLVertice.h
	GLWrapper.cpp
	GLWrapper.h
	image.cpp
	image.h
	material.cpp
	material.h
	object.cpp
	object.h
	pkg.cpp
	pkg.h
	teximage.cpp
	teximage.h
	wallpaper.cpp
	wallpaper.h
	WPJson.cpp
	WPJson.h
	WPRender.cpp
	WPRender.h
	WPShaderManager.cpp
	WPShaderManager.h
	WPTextureCache.cpp
	WPTextureCache.h
	FpsCounter.h
	FpsCounter.cpp
	FrameTimer.h
	FrameTimer.cpp
)
set_property(TARGET ${PROJECT_NAME} PROPERTY POSITION_INDEPENDENT_CODE ON)
target_link_libraries(${PROJECT_NAME}
    PUBLIC 
		OpenGL::GL
		${LZ4_LIBRARIES}
		${CMAKE_THREAD_LIBS_INIT}
		glad
)
target_include_directories(${PROJECT_NAME} PUBLIC .)
