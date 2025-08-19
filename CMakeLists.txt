cmake_minimum_required(VERSION 3.26)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

project(SokudoEngine)
file(GLOB_RECURSE SOURCES
        source/*.cpp
)
add_executable(SokudoEngine ${SOURCES}
        source/core/window/Window.cpp)

find_package(glfw3 CONFIG REQUIRED)
find_package(glm CONFIG REQUIRED)
find_package(Vulkan REQUIRED)
find_package(vk-bootstrap REQUIRED)
find_package(VulkanMemoryAllocator REQUIRED)
find_package(assimp REQUIRED)
find_package(imgui REQUIRED)
find_package(unofficial-spirv-reflect REQUIRED)

file(GLOB GLSL_SOURCE_FILES
        shaders/*.frag
        shaders/*.vert
)

if (Vulkan_GLSLC_EXECUTABLE)
    message("Using glslc to compile shaders")
    foreach (GLSL ${GLSL_SOURCE_FILES})
        get_filename_component(FILE_NAME ${GLSL} NAME)
        set(SPIRV "${CMAKE_SOURCE_DIR}/shaders/${FILE_NAME}.spv")
        add_custom_command(
                OUTPUT ${SPIRV}
                COMMAND ${Vulkan_GLSLC_EXECUTABLE} -o ${SPIRV} ${GLSL}
                DEPENDS ${GLSL})
        list(APPEND SPIRV_BINARY_FILES ${SPIRV})
    endforeach (GLSL)
elseif (Vulkan_GLSLANG_VALIDATOR_EXECUTABLE)
    message("Using glslangValidator to compile shaders")
    foreach (GLSL ${GLSL_SOURCE_FILES})
        get_filename_component(FILE_NAME ${GLSL} NAME)
        set(SPIRV "${CMAKE_SOURCE_DIR}/shaders/${FILE_NAME}.spv")
        add_custom_command(
                OUTPUT ${SPIRV}
                COMMAND ${Vulkan_GLSLANG_VALIDATOR_EXECUTABLE} -V -o ${SPIRV} ${GLSL}
                DEPENDS ${GLSL})
        list(APPEND SPIRV_BINARY_FILES ${SPIRV})
    endforeach (GLSL)
endif ()

include_directories(source)

set(DEPENDENCIES
        glfw
        glm::glm
        Vulkan::Vulkan
        vk-bootstrap::vk-bootstrap
        GPUOpen::VulkanMemoryAllocator
        assimp::assimp
        imgui::imgui
        unofficial::spirv-reflect
)

if (NOT MSVC)
    list(APPEND DEPENDENCIES stdc++)
endif ()

target_link_libraries(SokudoEngine PRIVATE ${DEPENDENCIES})