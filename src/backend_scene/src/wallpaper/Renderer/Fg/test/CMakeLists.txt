cmake_minimum_required(VERSION 3.13)
set(CMAKE_CXX_STANDARD 17)

project(test)

add_executable(${PROJECT_NAME}
	main.cpp
	../DependencyGraph.cpp
	../FrameGraph.cpp
)
target_link_libraries(${PROJECT_NAME})
target_include_directories(${PROJECT_NAME} PUBLIC . ..)