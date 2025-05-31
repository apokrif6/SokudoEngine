cmake_minimum_required(VERSION 3.26)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

project(SokudoEngine)
file(GLOB_RECURSE SOURCES
        main.cpp
        source/*.cpp
)
add_executable(SokudoEngine ${SOURCES}
        source/core/window/Window.cpp)

if (WIN32)
    list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_LIST_DIR}/cmake")
endif ()

find_package(glfw3 3.3 REQUIRED)
find_package(Vulkan REQUIRED)
find_package(vk-bootstrap REQUIRED)
find_package(VulkanMemoryAllocator REQUIRED)
find_package(assimp REQUIRED)
find_package(imgui REQUIRED)
find_package(unofficial-spirv-reflect REQUIRED)

if (UNIX)
    set(GLFW3_LIBRARY glfw)
endif ()

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

# copy shaders
add_custom_target(
        Shaders
        DEPENDS ${SPIRV_BINARY_FILES}
)
add_dependencies(SokudoEngine Shaders)

add_custom_command(TARGET Shaders POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory
        "$<TARGET_PROPERTY:SokudoEngine,SOURCE_DIR>/shaders"
        "$<TARGET_PROPERTY:SokudoEngine,BINARY_DIR>/shaders"
)

# copy textures
file(GLOB TEX_SOURCE_FILES
        textures/*
)

add_custom_target(
        Textures
        DEPENDS ${TEX_SOURCE_FILES}
)
add_dependencies(SokudoEngine Textures)

add_custom_command(TARGET Textures POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory
        "$<TARGET_PROPERTY:SokudoEngine,SOURCE_DIR>/textures"
        "$<TARGET_PROPERTY:SokudoEngine,BINARY_DIR>/textures"
)

# copy assets
file(GLOB ASSET_SOURCE_FILES
        assets/*
)

add_custom_target(
        Assets
        DEPENDS ${ASSET_SOURCE_FILES}
)

add_dependencies(SokudoEngine Assets)

add_custom_command(TARGET Assets POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory
        "$<TARGET_PROPERTY:SokudoEngine,SOURCE_DIR>/assets"
        "$<TARGET_PROPERTY:SokudoEngine,BINARY_DIR>/assets"
)

# includes
include_directories(${GLFW3_INCLUDE_DIR} source)

set(DEPENDENCIES
        ${GLFW3_LIBRARY}
        Vulkan::Vulkan
        vk-bootstrap::vk-bootstrap
        GPUOpen::VulkanMemoryAllocator
        assimp::assimp
        imgui::imgui
        unofficial::spirv-reflect
)

if (NOT MSVC)
    list(APPEND DEPENDENCIES stdc++ m)
endif ()

target_link_libraries(SokudoEngine PRIVATE ${DEPENDENCIES})