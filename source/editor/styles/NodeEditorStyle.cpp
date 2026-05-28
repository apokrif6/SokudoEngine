#include "NodeEditorStyle.h"
#include <imgui_node_editor.h>

void Editor::UI::NodeEditorStyle::push()
{
    ax::NodeEditor::Style& editorStyle = ax::NodeEditor::GetStyle();
    const ImGuiStyle& imguiStyle = ImGui::GetStyle();

    editorStyle.NodeRounding = 6.f;
    editorStyle.NodePadding = ImVec4(10.f, 10.f, 10.f, 10.f);
    editorStyle.NodeBorderWidth = 1.0f;
    editorStyle.HoveredNodeBorderWidth = 1.5f;
    editorStyle.SelectedNodeBorderWidth = 2.0f;

    editorStyle.PinRounding = 4.0f;
    editorStyle.PinBorderWidth = 1.0f;
    editorStyle.LinkStrength = 100.0f;

    const ImVec4 deepBackground = imguiStyle.Colors[ImGuiCol_WindowBg];
    const ImVec4 panelBackground = imguiStyle.Colors[ImGuiCol_ChildBg];
    const ImVec4 border = imguiStyle.Colors[ImGuiCol_Border];

    const ImVec4 violetDark = imguiStyle.Colors[ImGuiCol_Button];
    const ImVec4 violetMain = imguiStyle.Colors[ImGuiCol_ButtonHovered];
    const ImVec4 violetNeon = imguiStyle.Colors[ImGuiCol_ButtonActive];

    editorStyle.Colors[ax::NodeEditor::StyleColor_Bg] = deepBackground;
    editorStyle.Colors[ax::NodeEditor::StyleColor_Grid] = ImVec4(border.x, border.y, border.z, 0.15f);

    editorStyle.Colors[ax::NodeEditor::StyleColor_NodeBg] = panelBackground;
    editorStyle.Colors[ax::NodeEditor::StyleColor_NodeBorder] = border;
    editorStyle.Colors[ax::NodeEditor::StyleColor_HovNodeBorder] = violetDark;
    editorStyle.Colors[ax::NodeEditor::StyleColor_SelNodeBorder] = violetNeon;

    editorStyle.Colors[ax::NodeEditor::StyleColor_NodeSelRect] =
        ImVec4(violetMain.x, violetMain.y, violetMain.z, 0.15f);
    editorStyle.Colors[ax::NodeEditor::StyleColor_NodeSelRectBorder] = violetNeon;

    editorStyle.Colors[ax::NodeEditor::StyleColor_HovLinkBorder] = violetDark;
    editorStyle.Colors[ax::NodeEditor::StyleColor_SelLinkBorder] = violetNeon;
    editorStyle.Colors[ax::NodeEditor::StyleColor_LinkSelRect] =
        ImVec4(violetMain.x, violetMain.y, violetMain.z, 0.15f);
    editorStyle.Colors[ax::NodeEditor::StyleColor_LinkSelRectBorder] = violetNeon;

    editorStyle.Colors[ax::NodeEditor::StyleColor_PinRect] =
        ImVec4(panelBackground.x, panelBackground.y, panelBackground.z, 1.f);
    editorStyle.Colors[ax::NodeEditor::StyleColor_PinRectBorder] = violetMain;

    editorStyle.Colors[ax::NodeEditor::StyleColor_Flow] = violetNeon;
    editorStyle.Colors[ax::NodeEditor::StyleColor_FlowMarker] = violetNeon;

    editorStyle.Colors[ax::NodeEditor::StyleColor_GroupBg] =
        ImVec4(deepBackground.x, deepBackground.y, deepBackground.z, 0.6f);
    editorStyle.Colors[ax::NodeEditor::StyleColor_GroupBorder] = border;

    ImGui::PushStyleColor(ImGuiCol_FrameBg, imguiStyle.Colors[ImGuiCol_FrameBg]);
}

void Editor::UI::NodeEditorStyle::pop() { ImGui::PopStyleColor(1); }