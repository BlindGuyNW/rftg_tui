cmake_minimum_required(VERSION 3.10)

project(rftg)

if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
    message(STATUS "Setting build type to 'Debug' as none was specified.")
    set(CMAKE_BUILD_TYPE Debug CACHE STRING "Choose the type of build." FORCE)
endif()

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/${CMAKE_BUILD_TYPE})


# Add your C source files here
set(SOURCES
    rftg.c
    ai.c
    net.c
    tui.c
    engine.c
    init.c
    loadsave.c
    # ... and so on
)
add_executable(${PROJECT_NAME} ${SOURCES})
if(MSVC)
    target_compile_options(${PROJECT_NAME} PRIVATE /wd4996)
endif()

# Link against the math library on Linux
if(UNIX AND NOT APPLE)
    target_link_libraries(${PROJECT_NAME} m)
endif()
