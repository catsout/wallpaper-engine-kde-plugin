find_package(Qt5 COMPONENTS Quick Qml REQUIRED)
add_subdirectory(backend/mpv)
add_subdirectory(backend/scene)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

add_library(${PROJECT_NAME}
	SHARED
	plugin.cpp
	WPCommon.h
	MouseGrabber.h
	MouseGrabber.cpp
	qmldir
)

target_link_libraries(${PROJECT_NAME} 
	PRIVATE
	Qt5::Quick
	Qt5::Qml 
	mpvbackend
	scenebackend
)

# During plugin searching Qt will be looking for 'qmldir' file
# So we should place it next to our plugin lib.
add_custom_command(
    TARGET ${PROJECT_NAME}
    POST_BUILD
    COMMAND
        ${CMAKE_COMMAND} -E copy
        ${CMAKE_CURRENT_LIST_DIR}/qmldir
		$<TARGET_FILE_DIR:${PROJECT_NAME}>/qmldir
)

install(
	TARGETS ${PROJECT_NAME}
    DESTINATION ${QML_INSTALL_DIR}/${QMLPLUGIN_INSTALL_URI}
)

install(FILES
    ${CMAKE_CURRENT_BINARY_DIR}/qmldir
    DESTINATION ${QML_INSTALL_DIR}/${QMLPLUGIN_INSTALL_URI}
)

