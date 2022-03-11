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

include_directories(../third_party)

if(BUILD_QML)
    add_subdirectory(.. scenebackend)
else()
    add_subdirectory(../third_party third_party)
    add_subdirectory(../wallpaper wallpaper)
endif()

pkg_check_modules(GLFW REQUIRED glfw3)
add_executable(${PROJECT_NAME}
    glfwviewer.cpp
)
target_link_libraries(${PROJECT_NAME}
    PUBLIC 
        ${GLFW_LIBRARIES}
        wallpaper
)

if(BUILD_QML)
    find_package(Qt5 COMPONENTS Qml REQUIRED)

    set(CMAKE_AUTOMOC ON) 
    set(CMAKE_AUTORCC ON) 
    set(CMAKE_AUTOUIC ON)

    add_executable(${PROJECT_NAME}-qml
        qmlviewer.cpp
        main.qml
        pkg.qrc
    )
    target_link_libraries(${PROJECT_NAME}-qml
        PUBLIC 
            scenebackend
    )
    set(CMAKE_AUTOMOC OFF) 
    set(CMAKE_AUTORCC OFF) 
    set(CMAKE_AUTOUIC OFF)
endif()
