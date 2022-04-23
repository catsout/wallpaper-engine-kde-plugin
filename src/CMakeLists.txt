find_package(Qt5 COMPONENTS Quick Qml REQUIRED)
add_subdirectory(backend_mpv)
add_subdirectory(backend_scene)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

add_library(${PROJECT_NAME}
	SHARED
	plugin.cpp
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
set_target_properties(${PROJECT_NAME} PROPERTIES CXX_VISIBILITY_PRESET hidden)
set_target_properties(${PROJECT_NAME}
                       PROPERTIES LINK_FLAGS "-Wl,--exclude-libs,ALL")

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
    DESTINATION ${KDE_INSTALL_QMLDIR}/${QMLPLUGIN_INSTALL_URI}
)

install(FILES
    ${CMAKE_CURRENT_BINARY_DIR}/qmldir
    DESTINATION ${KDE_INSTALL_QMLDIR}/${QMLPLUGIN_INSTALL_URI}
)

