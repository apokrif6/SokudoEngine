cmake_minimum_required(VERSION 3.26)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

project(SokudoEngine)
file(GLOB_RECURSE SOURCES
        source/*.cpp
)
add_executable(SokudoEngine ${SOURCES})

find_package(glfw3 CONFIG REQUIRED)
find_package(glm CONFIG REQUIRED)
find_package(Vulkan REQUIRED)
find_package(vk-bootstrap REQUIRED)
find_package(VulkanMemoryAllocator REQUIRED)
find_package(assimp REQUIRED)
find_package(imgui REQUIRED)
find_package(imguizmo CONFIG REQUIRED)
find_package(unofficial-spirv-reflect REQUIRED)
find_package(yaml-cpp REQUIRED)

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

add_custom_target(Shaders ALL DEPENDS ${SPIRV_BINARY_FILES})
add_dependencies(SokudoEngine Shaders)

include_directories(source)

set(DEPENDENCIES
        glfw
        glm::glm
        Vulkan::Vulkan
        vk-bootstrap::vk-bootstrap
        GPUOpen::VulkanMemoryAllocator
        assimp::assimp
        imgui::imgui
        imguizmo::imguizmo
        unofficial::spirv-reflect
        yaml-cpp
)

if (NOT MSVC)
    list(APPEND DEPENDENCIES stdc++)
endif ()

target_link_libraries(SokudoEngine PRIVATE ${DEPENDENCIES})