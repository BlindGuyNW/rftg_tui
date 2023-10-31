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

# Link against the math library on Linux
if(UNIX AND NOT APPLE)
    target_link_libraries(${PROJECT_NAME} m)
endif()
# Copy cards and campaigns into the directory. into 
file(COPY ${CMAKE_SOURCE_DIR}/cards.txt DESTINATION ${CMAKE_BINARY_DIR})
file(COPY ${CMAKE_SOURCE_DIR}/campaign.txt DESTINATION ${CMAKE_BINARY_DIR})
file(COPY ${CMAKE_SOURCE_DIR}/network/ DESTINATION ${CMAKE_BINARY_DIR}/network)
