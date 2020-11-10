find_package(Qt5 COMPONENTS Gui Quick X11Extras REQUIRED)
include_directories(${Qt5Gui_PRIVATE_INCLUDE_DIRS})

if(PKG_CONFIG_FOUND)
  pkg_check_modules(PC_MPV QUIET mpv)
endif(PKG_CONFIG_FOUND)

project(mpvbackend)
add_library(${PROJECT_NAME}
	mpvbackend.cpp  
	mpvbackend.h  
	qthelper.hpp
)
target_link_libraries(${PROJECT_NAME} 
    PUBLIC 
        Qt5::Quick
        Qt5::Gui
		Qt5::X11Extras
		mpv
)
target_include_directories(${PROJECT_NAME} PUBLIC .) #${INCLUDE_DIRECTORIES})
