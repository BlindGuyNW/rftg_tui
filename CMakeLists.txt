cmake_minimum_required(VERSION 3.10)

project(rftg)

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
                   COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_SOURCE_DIR}/cards.txt ${CMAKE_BINARY_DIR}/$<CONFIG>
                   COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_SOURCE_DIR}/campaign.txt ${CMAKE_BINARY_DIR}/$<CONFIG>
                   COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_SOURCE_DIR}/network/ ${CMAKE_BINARY_DIR}/$<CONFIG>/network)
