project(scenebackend)
find_package(Qt5 COMPONENTS Gui Quick Qml X11Extras REQUIRED)
include_directories(${Qt5Gui_PRIVATE_INCLUDE_DIRS})
include_directories(include)

add_subdirectory(lib/glad)
add_subdirectory(src/wallpaper)

set(CMAKE_AUTOMOC ON) 
set(CMAKE_AUTORCC ON) 
set(CMAKE_AUTOUIC ON)

add_library(${PROJECT_NAME}
	src/SceneViewer.cpp
	src/SceneViewer.h
)
target_link_libraries(${PROJECT_NAME}
    PUBLIC 
        Qt5::Quick
        Qt5::Gui
		Qt5::X11Extras
		wallpaper
)
target_include_directories(${PROJECT_NAME} PUBLIC src)


add_executable(${PROJECT_NAME}-viewer-qml
	src/viewer/qmlviewer.cpp
	src/viewer/main.qml
	src/viewer/pkg.qrc
)
target_link_libraries(${PROJECT_NAME}-viewer-qml
    PUBLIC 
        Qt5::Quick
        Qt5::Gui
		${PROJECT_NAME}
)

set(CMAKE_AUTOMOC OFF) 
set(CMAKE_AUTORCC OFF) 
set(CMAKE_AUTOUIC OFF) 

pkg_check_modules(GLFW QUIET glfw3)

if (GLFW_FOUND)
	add_executable(${PROJECT_NAME}-viewer
		src/viewer/glfwviewer.cpp
	)
	target_link_libraries(${PROJECT_NAME}-viewer
		PUBLIC 
			${GLFW_LIBRARIES}
			${PROJECT_NAME}
	)
endif ()
