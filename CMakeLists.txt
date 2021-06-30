cmake_minimum_required(VERSION 3.13)
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

project("WallpaperEngineKde")

find_program(QMAKE qmake)
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
