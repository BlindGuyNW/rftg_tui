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
# Copy cards and campaigns into the directory. into which the final exeuctable is places.
# Note that the copy command doesn't work in all cases, so we use some slightly weird custom command logic.
add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
                   COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_SOURCE_DIR}/cards.txt ${CMAKE_BINARY_DIR}/$<$<BOOL:${CMAKE_CONFIGURATION_TYPES}>:$<CONFIG>>$<$<NOT:$<BOOL:${CMAKE_CONFIGURATION_TYPES}>>:${CMAKE_BUILD_TYPE}>
                   COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_SOURCE_DIR}/campaign.txt ${CMAKE_BINARY_DIR}/$<$<BOOL:${CMAKE_CONFIGURATION_TYPES}>:$<CONFIG>>$<$<NOT:$<BOOL:${CMAKE_CONFIGURATION_TYPES}>>:${CMAKE_BUILD_TYPE}>
                   COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_SOURCE_DIR}/network/ ${CMAKE_BINARY_DIR}/$<$<BOOL:${CMAKE_CONFIGURATION_TYPES}>:$<CONFIG>>$<$<NOT:$<BOOL:${CMAKE_CONFIGURATION_TYPES}>>:${CMAKE_BUILD_TYPE}>/network)
