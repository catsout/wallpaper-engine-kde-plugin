project(wescene-renderer)

find_package(PkgConfig REQUIRED)


include_directories(third_party)
add_subdirectory(third_party)
add_subdirectory(src)


if(BUILD_QML)
    find_package(Qt5 COMPONENTS Gui Quick Qml REQUIRED)

    set(CMAKE_AUTOMOC ON) 
    set(CMAKE_AUTORCC ON) 
    set(CMAKE_AUTOUIC ON)

    add_library(${PROJECT_NAME}-qml
    STATIC
        qml_helper/SceneBackend.cpp
        qml_helper/glExtra.cpp
    )
    target_link_libraries(${PROJECT_NAME}-qml
    PUBLIC
        Qt5::Quick
        Qt5::Gui
        glad
        ${PROJECT_NAME}
    )
    target_include_directories(${PROJECT_NAME}-qml PUBLIC qml_helper third_party)

    set(CMAKE_AUTOMOC OFF) 
    set(CMAKE_AUTORCC OFF) 
    set(CMAKE_AUTOUIC OFF) 
endif()
