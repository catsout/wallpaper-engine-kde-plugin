project(mpvbackend)

find_package(Qt5 COMPONENTS Gui Quick X11Extras REQUIRED)
include_directories(${Qt5Gui_PRIVATE_INCLUDE_DIRS})

find_package(PkgConfig REQUIRED)
pkg_check_modules(MPV REQUIRED mpv)

set(CMAKE_AUTOMOC ON) 
set(CMAKE_AUTORCC ON) 
set(CMAKE_AUTOUIC ON) 

add_library(${PROJECT_NAME}
	STATIC
	MpvBackend.cpp  
	qthelper.hpp
)
target_link_libraries(${PROJECT_NAME} 
	PRIVATE 
    Qt5::Quick
	Qt5::Gui
	Qt5::X11Extras
	${MPV_LIBRARIES}
)
target_include_directories(${PROJECT_NAME} PUBLIC .) #${INCLUDE_DIRECTORIES})
