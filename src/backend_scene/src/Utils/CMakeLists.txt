set(LIB_NAME wpUtils)

add_library(${LIB_NAME}
STATIC
Logging.cpp
FpsCounter.cpp
Algorism.cpp	
Sha.cpp
DynamicLibrary.cpp
)

target_link_libraries(${LIB_NAME}
PRIVATE
)
target_include_directories(${LIB_NAME} PUBLIC include PRIVATE include/Utils)
target_compile_options(${LIB_NAME} PRIVATE ${warn_opts})
set_property(TARGET ${LIB_NAME} PROPERTY POSITION_INDEPENDENT_CODE ON)
