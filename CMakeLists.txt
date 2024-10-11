cmake_minimum_required(VERSION 3.26)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

project(SokudoEngine)
file(GLOB SOURCES
        Main.cpp
        window/*.cpp
        tools/*.cpp
        vk-renderer/*.cpp
)
add_executable(SokudoEngine ${SOURCES})

if (WIN32)
    list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_LIST_DIR}/cmake")
endif ()

find_package(glfw3 3.3 REQUIRED)
find_package(Vulkan REQUIRED)

if (UNIX)
    set(GLFW3_LIBRARY glfw)
endif ()

add_subdirectory(vk-bootstrap)

include_directories(${GLFW3_INCLUDE_DIR} window tools vk-renderer vk-memory-allocation)

target_link_libraries(SokudoEngine ${GLFW3_LIBRARY} Vulkan::Vulkan vk-bootstrap::vk-bootstrap)

