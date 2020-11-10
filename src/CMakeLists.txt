find_package(Qt5 COMPONENTS Qml REQUIRED)
add_subdirectory(backend/mpv)

add_library(${PROJECT_NAME} SHARED
	plugin.cpp
	viewModel.cpp
	viewModel.h
	weProjectLoader.cpp
	weProjectLoader.h
	qmldir
)

target_link_libraries(${PROJECT_NAME} 
	PUBLIC 
		Qt5::Qml 
		mpvbackend
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
    DESTINATION ${QT_INSTALL_QML}/${QMLPLUGIN_INSTALL_URI}
)

install(FILES
    ${CMAKE_CURRENT_BINARY_DIR}/qmldir
    DESTINATION ${QT_INSTALL_QML}/${QMLPLUGIN_INSTALL_URI}
)

