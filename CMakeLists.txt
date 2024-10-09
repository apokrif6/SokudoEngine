cmake_minimum_required(VERSION 3.26)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

project(SokudoEngine)
file(GLOB SOURCES
        Main.cpp
        window/*.cpp
        tools/*.cpp
)
add_executable(SokudoEngine ${SOURCES})

if (WIN32)
    list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_LIST_DIR}/cmake")
endif ()

find_package(glfw3 3.3 REQUIRED)

if (UNIX)
    set(GLFW3_LIBRARY glfw)
endif ()

include_directories(${GLFW3_INCLUDE_DIR} window tools)

target_link_libraries(SokudoEngine ${GLFW3_LIBRARY})

