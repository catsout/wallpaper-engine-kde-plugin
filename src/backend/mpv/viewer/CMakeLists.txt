cmake_minimum_required(VERSION 3.13)
set(CMAKE_CXX_STANDARD 14) 
set(CMAKE_CXX_STANDARD_REQUIRED ON) 
project(mpvbackend-viewer)

find_package(Qt5 COMPONENTS Gui Quick REQUIRED)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

add_subdirectory(.. mpvbackend)

add_executable(${PROJECT_NAME}
	qmlviewer.cpp
	main.qml
	qml.qrc
)
target_link_libraries(${PROJECT_NAME} 
    PUBLIC 
		mpvbackend
		Qt::Gui
		Qt::Quick
)
