cmake_minimum_required(VERSION 3.16)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(KF_MIN_VERSION "5.68.0")

project("WallpaperEngineKde")


set(CMAKE_SKIP_INSTALL_ALL_DEPENDENCY true)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)
if(NOT OpenGL_GL_PREFERENCE)
  set(OpenGL_GL_PREFERENCE "GLVND")
endif()


option(USE_PLASMAPKG "Use plasmapkg2 to install plugin to user'home"
       ${USE_PLASMAPKG})

set(DEFAULT_BUILD_TYPE "RelWithDebInfo")
if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
  message(
    STATUS
      "Setting build type to '${DEFAULT_BUILD_TYPE}' as none was specified.")
  set(CMAKE_BUILD_TYPE
      "${DEFAULT_BUILD_TYPE}"
      CACHE STRING "Choose the type of build." FORCE)
endif()

if(CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
  set(CMAKE_INSTALL_PREFIX
      /usr
      CACHE PATH "set prefix to /usr" FORCE)
endif(CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)


find_package(ECM REQUIRED NO_MODULE)
set(CMAKE_MODULE_PATH ${ECM_MODULE_PATH} ${CMAKE_MODULE_PATH})

if(ECM_VERSION_MAJOR EQUAL 6)
  find_package(Plasma REQUIRED)
  find_package(KF6 ${KF_MIN_VERSION} REQUIRED COMPONENTS Package)
  include(KDEInstallDirs6)
else()
  find_package(KF5 ${KF_MIN_VERSION} REQUIRED COMPONENTS Package Plasma)
  include(KDEInstallDirs)
endif()

if(NOT DEFINED QT_MAJOR_VERSION)
message(FATAL_ERROR "Qt not found")
endif()

if(NOT USE_PLASMAPKG)
  plasma_install_package(plugin com.github.catsout.wallpaperEngineKde wallpapers
                         wallpaper)
else()
  find_program(KPACKAGETOOL NAMES kpackagetool6 kpackagetool)
  if(${KPACKAGETOOL} STREQUAL "KPACKAGETOOL-NOTFOUND")
    message(FATAL_ERROR "kpackagetool not found")
  endif()

  add_custom_target(
    install_pkg
    COMMAND /bin/sh -c
            "${KPACKAGETOOL} -t Plasma/Wallpaper -i ./plugin 2>/dev/null || ${KPACKAGETOOL} -t Plasma/Wallpaper -u ./plugin"
    WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}
    VERBATIM)
endif()

set(QMLPLUGIN_URI "com.github.catsout.wallpaperEngineKde")
string(REPLACE "." "/" QMLPLUGIN_INSTALL_URI ${QMLPLUGIN_URI})

add_subdirectory(src)
