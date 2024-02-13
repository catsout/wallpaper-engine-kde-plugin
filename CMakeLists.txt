cmake_minimum_required(VERSION 3.16)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(KF5_MIN_VERSION "5.68.0")

project("WallpaperEngineKde")

set(CMAKE_SKIP_INSTALL_ALL_DEPENDENCY true)

option(USE_PLASMAPKG "Use plasmapkg2 to install plugin to user'home" ${USE_PLASMAPKG})

set(DEFAULT_BUILD_TYPE "RelWithDebInfo")
if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
    message(STATUS "Setting build type to '${DEFAULT_BUILD_TYPE}' as none was specified.")
    set(CMAKE_BUILD_TYPE "${DEFAULT_BUILD_TYPE}" CACHE STRING "Choose the type of build." FORCE)
endif()

IF(CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
  SET(CMAKE_INSTALL_PREFIX /usr CACHE PATH "set prefix to /usr" FORCE)
ENDIF(CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)

set(CMAKE_POSITION_INDEPENDENT_CODE ON)

if(NOT OpenGL_GL_PREFERENCE) 
set(OpenGL_GL_PREFERENCE "GLVND")
endif()

find_package(ECM REQUIRED NO_MODULE)
set(CMAKE_MODULE_PATH ${ECM_MODULE_PATH} ${CMAKE_MODULE_PATH})
include(KDEInstallDirs)
if(QT_MAJOR_VERSION EQUAL 6)
find_package(KF6 ${KF5_MIN_VERSION} REQUIRED COMPONENTS
    Package
)
find_package(Plasma)
else()
find_package(KF5 ${KF5_MIN_VERSION} REQUIRED COMPONENTS
    Package
    Plasma
)
endif()

if(NOT USE_PLASMAPKG)
    plasma_install_package(plugin com.github.casout.wallpaperEngineKde wallpapers wallpaper)
else()
    find_program(PLASMAPKG2 NAMES plasmapkg2)
    if(${PLASMAPKG2} STREQUAL "PLASMAPKG2-NOTFOUND")
        message(FATAL_ERROR "plasmapkg2 not found")
    endif()

    add_custom_target(install_pkg
        COMMAND /bin/sh -c "${PLASMAPKG2} -i ./plugin 2>/dev/null || ${PLASMAPKG2} -u ./plugin"
        WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}
        VERBATIM
    )
endif()

set(QMLPLUGIN_URI "com.github.catsout.wallpaperEngineKde")
string(REPLACE "." "/" QMLPLUGIN_INSTALL_URI ${QMLPLUGIN_URI})

add_subdirectory(src)
