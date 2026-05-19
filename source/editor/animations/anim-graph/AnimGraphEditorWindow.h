#pragma once

#include "animations/anim-graph/AnimGraph.h"
#include "imgui_node_editor.h"
#include "editor/popup/PopupRequest.h"

namespace Core::Component
{
class MeshComponent;
}
// TODO
// use CRTP pattern for UIWindow<>, but with state management
namespace Editor::Animations
{
struct EditorNodeData
{
    uint64_t NodeId;
    uint64_t InputPinId;
    uint64_t OutputPinId;
};
class AnimGraphEditorWindow
{
public:
    static void draw();

    static void open(Core::Component::MeshComponent* meshComponent);

private:
    static void CreateEditorNode(const uuids::uuid& uuid);

    inline static bool mIsOpen = false;
    inline static Core::Component::MeshComponent* mCurrentMeshComponent = nullptr;

    inline static std::unordered_map<uuids::uuid, EditorNodeData> mEditorNodes{};
    inline static uint64_t mNextId = 1;

    inline static UI::PopupRequest mClipSelectorPopup{};
    inline static bool mClipSelectorPopupOpen = false;
    inline static uuids::uuid mClipSelectorPopupOpenNode;
};
} // namespace Editor::Animations