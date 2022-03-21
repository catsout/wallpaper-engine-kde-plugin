project(scenebackend)
find_package(Qt5 COMPONENTS Gui Quick Qml REQUIRED)

find_package(PkgConfig REQUIRED)


include_directories(third_party)
add_subdirectory(third_party)
add_subdirectory(wallpaper)

set(CMAKE_AUTOMOC ON) 
set(CMAKE_AUTORCC ON) 
set(CMAKE_AUTOUIC ON)

add_library(${PROJECT_NAME}
STATIC
	SceneBackend.cpp
    glExtra.cpp
)
target_link_libraries(${PROJECT_NAME}
PUBLIC
	Qt5::Quick
	Qt5::Gui
	glad
	wallpaper
)
target_include_directories(${PROJECT_NAME} PUBLIC . third_party)

set(CMAKE_AUTOMOC OFF) 
set(CMAKE_AUTORCC OFF) 
set(CMAKE_AUTOUIC OFF) 
