cmake_minimum_required(VERSION 3.13)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

project("WallpaperEngineKde")

set(DEFAULT_BUILD_TYPE "Release")
if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
    message(STATUS "Setting build type to '${DEFAULT_BUILD_TYPE}' as none was specified.")
    set(CMAKE_BUILD_TYPE "${DEFAULT_BUILD_TYPE}" CACHE STRING "Choose the type of build." FORCE)
endif()

find_program(QMAKE NAMES qmake-qt5 qmake)
if(${QMAKE} STREQUAL "QMAKE-NOTFOUND")
	message(FATAL_ERROR "qmake not found, make sure you have it installed")
endif()

execute_process(
    COMMAND ${QMAKE} -query QT_INSTALL_QML
    OUTPUT_VARIABLE PROC_RESULT
    OUTPUT_STRIP_TRAILING_WHITESPACE
)
set(QT_INSTALL_QML ${PROC_RESULT})
set(QMLPLUGIN_URI "com.github.catsout.wallpaperEngineKde")
string(REPLACE "." "/" QMLPLUGIN_INSTALL_URI ${QMLPLUGIN_URI})

add_subdirectory(src)
