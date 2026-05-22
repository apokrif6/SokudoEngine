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
struct EditorPinData
{
    uint64_t EditorId = 0;
    Core::Animations::PinID RuntimePinId{};
};
struct EditorNodeData
{
    uint64_t NodeId;
    std::vector<EditorPinData> InputPins;
    std::vector<EditorPinData> OutputPins;
};
struct EditorLinkData
{
    uint64_t linkId = 0;
    Core::Animations::PinID StartPinId{};
    Core::Animations::PinID EndPinId{};
};

class AnimGraphEditorWindow
{
public:
    static void draw();

    static void open(Core::Component::MeshComponent* meshComponent);

private:
    static void createEditorNode(const uuids::uuid& uuid);

    static const uuids::uuid* findNodeByPin(Core::Animations::PinID editorPinId);

    static Core::Animations::PinID findRuntimePin(Core::Animations::PinID editorPinId);

    static bool containsPin(const std::vector<EditorPinData>& pins, uint64_t editorPinId);

    static void removeLinksConnectedToPin(Core::Animations::PinID editorPinId, Core::Animations::AnimGraph* animGraph);

    static uint64_t generateEditorId();

    inline static bool mIsOpen = false;
    inline static Core::Component::MeshComponent* mCurrentMeshComponent = nullptr;

    inline static std::unordered_map<uuids::uuid, EditorNodeData> mEditorNodes{};
    inline static std::vector<EditorLinkData> mEditorLinks{};
    inline static uint64_t mNextEditorId = 1;

    inline static UI::PopupRequest mClipSelectorPopup{};
    inline static bool mClipSelectorPopupOpen = false;
    inline static uuids::uuid mClipSelectorPopupOpenNode;
};
} // namespace Editor::Animations