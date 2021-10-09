cmake_minimum_required(VERSION 3.13)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

project("sceneviewer")

set(DEFAULT_BUILD_TYPE "Release")
if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
    message(STATUS "Setting build type to '${DEFAULT_BUILD_TYPE}' as none was specified.")
    set(CMAKE_BUILD_TYPE "${DEFAULT_BUILD_TYPE}" CACHE STRING "Choose the type of build." FORCE)
endif()

option(BUILD_QML "Build qml scene viewer" ${BUILD_QML})


find_package(PkgConfig REQUIRED)
pkg_check_modules(GLFW REQUIRED glfw3)

include_directories(../../include)
add_subdirectory(../../lib/glad glad)
add_subdirectory(../wallpaper wallpaper)

add_executable(${PROJECT_NAME}
    glfwviewer.cpp
)
target_link_libraries(${PROJECT_NAME}
    PUBLIC 
        ${GLFW_LIBRARIES}
        wallpaper
)

if(BUILD_QML)
    find_package(Qt5 COMPONENTS Gui Quick Qml X11Extras REQUIRED)
    include_directories(${Qt5Gui_PRIVATE_INCLUDE_DIRS})

    include_directories(..)
    set(CMAKE_AUTOMOC ON) 
    set(CMAKE_AUTORCC ON) 
    set(CMAKE_AUTOUIC ON)

    add_executable(${PROJECT_NAME}-qml
        qmlviewer.cpp
        main.qml
        pkg.qrc
        ../SceneViewer.cpp
    )
    target_link_libraries(${PROJECT_NAME}-qml
        PUBLIC 
		    Qt5::Qml 
            Qt5::Quick
            Qt5::Gui
		    Qt5::X11Extras
            wallpaper
    )
    set(CMAKE_AUTOMOC OFF) 
    set(CMAKE_AUTORCC OFF) 
    set(CMAKE_AUTOUIC OFF)
endif()