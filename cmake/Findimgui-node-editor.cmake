if (TARGET imgui_node_editor::imgui_node_editor)
    return()
endif ()

find_package(imgui REQUIRED)
get_target_property(_IMGUI_INC imgui::imgui INTERFACE_INCLUDE_DIRECTORIES)

set(IMGUI_NODE_EDITOR_ROOT_DIR "${_IMGUI_INC}/imgui-node-editor")

set(imgui-node-editor-Sources
        ${IMGUI_NODE_EDITOR_ROOT_DIR}/crude_json.h
        ${IMGUI_NODE_EDITOR_ROOT_DIR}/imgui_bezier_math.h
        ${IMGUI_NODE_EDITOR_ROOT_DIR}/imgui_bezier_math.inl
        ${IMGUI_NODE_EDITOR_ROOT_DIR}/imgui_canvas.h
        ${IMGUI_NODE_EDITOR_ROOT_DIR}/imgui_extra_math.h
        ${IMGUI_NODE_EDITOR_ROOT_DIR}/imgui_extra_math.inl
        ${IMGUI_NODE_EDITOR_ROOT_DIR}/imgui_node_editor_internal.h
        ${IMGUI_NODE_EDITOR_ROOT_DIR}/imgui_node_editor_internal.inl
        ${IMGUI_NODE_EDITOR_ROOT_DIR}/imgui_node_editor.h
)

add_library(imgui-node-editor STATIC ${imgui-node-editor-Sources})

target_include_directories(imgui-node-editor PUBLIC
        ${IMGUI_NODE_EDITOR_ROOT_DIR}
)

target_link_libraries(imgui-node-editor PUBLIC imgui::imgui)

add_library(imgui-node-editor::imgui-node-editor ALIAS imgui-node-editor)

source_group(TREE ${IMGUI_NODE_EDITOR_ROOT_DIR} FILES ${imgui-node-editor-Sources})

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(
        imgui-node-editor
        REQUIRED_VARS IMGUI_NODE_EDITOR_ROOT_DIR
)