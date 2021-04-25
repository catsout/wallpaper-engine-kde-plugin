cmake_minimum_required(VERSION 3.13)
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

project("WallpaperEngineKde")

execute_process(
    COMMAND qmake -query QT_INSTALL_QML
    OUTPUT_VARIABLE PROC_RESULT
    OUTPUT_STRIP_TRAILING_WHITESPACE
)
set(QT_INSTALL_QML ${PROC_RESULT})
set(QMLPLUGIN_URI "com.github.catsout.wallpaperEngineKde")
string(REPLACE "." "/" QMLPLUGIN_INSTALL_URI ${QMLPLUGIN_URI})

add_subdirectory(src)
