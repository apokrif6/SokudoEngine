if (TARGET imgui-node-editor::imgui-node-editor)
    return()
endif ()

find_package(imgui REQUIRED)
get_target_property(_IMGUI_INC imgui::imgui INTERFACE_INCLUDE_DIRECTORIES)

set(IMGUI_NODE_EDITOR_ROOT_DIR "${_IMGUI_INC}/imgui-node-editor")

find_library(IMGUI_NODE_EDITOR_LIB
        NAMES imgui-node-editor imgui_node_editor
        PATHS "${_IMGUI_INC}/../lib"
        "${_IMGUI_INC}/../debug/lib"
)

if (NOT IMGUI_NODE_EDITOR_LIB)
    message(FATAL_ERROR "Не удалось найти библиотеку imgui-node-editor в vcpkg!")
endif ()

add_library(imgui-node-editor STATIC IMPORTED GLOBAL)

set_target_properties(imgui-node-editor PROPERTIES
        IMPORTED_LOCATION "${IMGUI_NODE_EDITOR_LIB}"
        INTERFACE_INCLUDE_DIRECTORIES "${IMGUI_NODE_EDITOR_ROOT_DIR}"
)

target_link_libraries(imgui-node-editor INTERFACE imgui::imgui)

add_library(imgui-node-editor::imgui-node-editor ALIAS imgui-node-editor)

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(
        imgui-node-editor
        REQUIRED_VARS IMGUI_NODE_EDITOR_ROOT_DIR IMGUI_NODE_EDITOR_LIB
)