project(scenebackend)
find_package(Qt5 COMPONENTS Gui Quick Qml X11Extras REQUIRED)
include_directories(${Qt5Gui_PRIVATE_INCLUDE_DIRS})

find_package(PkgConfig REQUIRED)
include_directories(include)

add_subdirectory(lib/glad)

SET(SKIP_GLSLANG_INSTALL ON CACHE BOOL "")
SET(ENABLE_GLSLANG_BINARIES OFF CACHE BOOL "")
SET(ENABLE_SPVREMAPPER OFF CACHE BOOL "")
SET(ENABLE_GLSLANG_JS OFF CACHE BOOL "")
SET(NABLE_HLSL ON CACHE BOOL "")
add_subdirectory(lib/glslang)
add_subdirectory(src/wallpaper)

set(CMAKE_AUTOMOC ON) 
set(CMAKE_AUTORCC ON) 
set(CMAKE_AUTOUIC ON)

add_library(${PROJECT_NAME}
	STATIC
	src/SceneViewer.cpp
	src/SceneViewer.h
)
target_link_libraries(${PROJECT_NAME}
	PRIVATE
	Qt5::Quick
	Qt5::Gui
	Qt5::X11Extras
	PUBLIC
	wallpaper
)
target_include_directories(${PROJECT_NAME} PUBLIC src include)

add_executable(${PROJECT_NAME}-qml
	src/viewer/qmlviewer.cpp
	src/viewer/main.qml
	src/viewer/pkg.qrc
	src/SceneViewer.cpp
)
target_link_libraries(${PROJECT_NAME}-qml
	PUBLIC
	Qt5::Quick
	Qt5::Gui
	Qt5::X11Extras
	${PROJECT_NAME}
)

set(CMAKE_AUTOMOC OFF) 
set(CMAKE_AUTORCC OFF) 
set(CMAKE_AUTOUIC OFF) 
