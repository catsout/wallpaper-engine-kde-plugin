project(mpvbackend)

    # Qt 6.10 has split this into it's own CMake module, it's no longer included with Gui
    if (Qt6Gui_VERSION VERSION_GREATER_EQUAL "6.10.0")
        find_package(Qt6GuiPrivate ${REQUIRED_QT_VERSION} REQUIRED NO_MODULE)
        get_target_property(Qt6Gui_PRIVATE_INCLUDE_DIRS Qt6::GuiPrivate INTERFACE_INCLUDE_DIRECTORIES)

        # QtGui depends on private headers form QtCore as well
        find_package(Qt6CorePrivate ${REQUIRED_QT_VERSION} REQUIRED NO_MODULE)
        get_target_property(Qt6Core_PRIVATE_INCLUDE_DIRS Qt6::CorePrivate INTERFACE_INCLUDE_DIRECTORIES)

        # Currently we depend on the Qt6Gui_PRIVATE_INCLUDE_DIRS variable to selectively include
        # the needed headers, but it will fail to compile in 6.10 since the private headers exist
        # in separate directories. So we'll just shove them into the variable we expect for now.
        list(APPEND Qt6Gui_PRIVATE_INCLUDE_DIRS ${Qt6Core_PRIVATE_INCLUDE_DIRS})
    endif()


find_package(Qt${QT_MAJOR_VERSION} COMPONENTS Gui Quick REQUIRED)
include_directories(${Qt${QT_MAJOR_VERSION}Gui_PRIVATE_INCLUDE_DIRS})

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
    Qt::Quick
	Qt::Gui
	# Qt::X11Extras
	${MPV_LIBRARIES}
)
target_include_directories(${PROJECT_NAME} PUBLIC .) #${INCLUDE_DIRECTORIES})
