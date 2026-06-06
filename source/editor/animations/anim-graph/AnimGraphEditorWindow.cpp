#include "AnimGraphEditorWindow.h"

#include "imgui.h"
#include "animations/anim-graph/nodes/AnimGraphBlendNode.h"
#include "animations/anim-graph/nodes/AnimGraphClipNode.h"
#include "animations/anim-graph/nodes/AnimGraphIKNode.h"
#include "animations/anim-graph/nodes/AnimGraphOutputPoseNode.h"
#include "components/MeshComponent.h"
#include "editor/elements/Elements.h"
#include "editor/styles/NodeEditorStyle.h"
#include "engine/Engine.h"
#include "ui/inspector/animation/AnimationInspectorInverseKinematicsUIWindow.h"

namespace ed = ax::NodeEditor;

static ed::EditorContext* g_Context = nullptr;

void Editor::Animations::AnimGraphEditorWindow::open(Core::Component::MeshComponent* meshComponent)
{
    mCurrentMeshComponent = meshComponent;
    mIsOpen = true;

    if (!meshComponent)
    {
        return;
    }

    const auto graph = meshComponent->getAnimGraph();

    if (!graph)
    {
        return;
    }

    bool hasOutput = false;

    for (const auto& node : graph->getNodes() | std::views::values)
    {
        if (dynamic_cast<Core::Animations::AnimGraphOutputPoseNode*>(node.get()))
        {
            hasOutput = true;
            break;
        }
    }

    if (!hasOutput)
    {
        const auto output = graph->createNode<Core::Animations::AnimGraphOutputPoseNode>();

        graph->setOutputNode(output->getUUID());
    }
}

void Editor::Animations::AnimGraphEditorWindow::draw()
{
    const auto animGraph = mCurrentMeshComponent ? mCurrentMeshComponent->getAnimGraph() : nullptr;

    if (!mIsOpen || !animGraph)
    {
        return;
    }

    if (!g_Context)
    {
        ed::Config config;
        config.SettingsFile = "AnimGraphEditor.json";

        g_Context = ed::CreateEditor(&config);
    }

    ed::SetCurrentEditor(g_Context);

    ImGui::Begin("AnimGraph Editor", &mIsOpen);

    ed::Begin("AnimGraph");

    UI::NodeEditorStyle::push();

    const auto openPopupPosition = ImGui::GetMousePos();

    ed::Suspend();
    if (ed::ShowBackgroundContextMenu())
    {
        ImGui::OpenPopup("GraphContextMenu");
    }

    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(8, 8));

    if (ImGui::BeginPopup("GraphContextMenu"))
    {
        if (ImGui::MenuItem("Add Clip Node"))
        {
            const auto node = animGraph->createNode<Core::Animations::AnimGraphClipNode>();
            node->setProperty("clipIndex", 0);

            createEditorNode(node->getUUID());
            ed::SetNodePosition(mEditorNodes[node->getUUID()].NodeId, openPopupPosition);
        }

        if (ImGui::MenuItem("Add Blend Node"))
        {
            const auto node = animGraph->createNode<Core::Animations::AnimGraphBlendNode>();
            node->setProperty("alpha", 0.5f);

            createEditorNode(node->getUUID());
            ed::SetNodePosition(mEditorNodes[node->getUUID()].NodeId, openPopupPosition);
        }

        if (ImGui::MenuItem("Add IK Node"))
        {
            const auto node = animGraph->createNode<Core::Animations::AnimGraphIKNode>();

            createEditorNode(node->getUUID());
            ed::SetNodePosition(mEditorNodes[node->getUUID()].NodeId, openPopupPosition);
        }

        ImGui::EndPopup();
    }
    ImGui::PopStyleVar();
    ed::Resume();

    for (auto& [uuid, node] : animGraph->getNodes())
    {
        if (!node)
        {
            continue;
        }

        if (!mEditorNodes.contains(uuid))
        {
            createEditorNode(uuid);
        }

        const auto& editorData = mEditorNodes[uuid];

        ed::BeginNode(editorData.NodeId);

        ImGui::PushID(static_cast<int>(editorData.NodeId));

        if (dynamic_cast<Core::Animations::AnimGraphClipNode*>(node.get()))
        {
            ImGui::TextColored(ImVec4(0.3f, 1.f, 0.3f, 1.f), "Clip");
        }
        else if (dynamic_cast<Core::Animations::AnimGraphBlendNode*>(node.get()))
        {
            ImGui::TextColored(ImVec4(0.3f, 0.7f, 1.f, 1.f), "Blend");
        }
        else if (dynamic_cast<Core::Animations::AnimGraphIKNode*>(node.get()))
        {
            ImGui::TextColored(ImVec4(0.9f, 0.6f, 0.2f, 1.f), "IK Solver");
        }
        else if (dynamic_cast<Core::Animations::AnimGraphOutputPoseNode*>(node.get()))
        {
            ImGui::TextColored(ImVec4(0.8f, 0.4f, 1.f, 1.f), "Output Pose");
        }

        const float nodeContentWidth = ed::GetNodeSize(editorData.NodeId).x - ImGui::GetStyle().WindowPadding.x * 2.0f;

        UI::Elements::nodeSeparator(nodeContentWidth);

        ImGui::BeginGroup();

        ImGui::PushItemWidth(nodeContentWidth);

        if (const auto* clipNode = dynamic_cast<Core::Animations::AnimGraphClipNode*>(node.get()))
        {
            const auto& clips = mCurrentMeshComponent->getAnimations();
            int clipIndex = 0;
            if (auto* property = clipNode->getProperty("clipIndex"))
            {
                clipIndex = std::get<int>(*property);
            }
            if (clips.empty())
            {
                ImGui::TextDisabled("No animations loaded");
            }
            else
            {
                if (const char* currentLabel = clips[clipIndex].name.c_str(); ImGui::Button(currentLabel))
                {
                    mClipSelectorPopupOpen = true;
                    mClipSelectorPopupOpenNode = uuid;

                    mClipSelectorPopup.id = "ClipSelector";

                    mClipSelectorPopup.items.clear();
                    mClipSelectorPopup.items.reserve(clips.size());

                    for (auto& clip : clips)
                    {
                        mClipSelectorPopup.items.push_back(clip.name);
                    }

                    mClipSelectorPopup.onSelect = [uuid, animGraph](int index)
                    {
                        const auto it = animGraph->getNodes().find(uuid);
                        if (it == animGraph->getNodes().end())
                        {
                            return;
                        }

                        if (auto* targetClipNode = dynamic_cast<Core::Animations::AnimGraphClipNode*>(it->second.get()))
                        {
                            targetClipNode->setProperty("clipIndex", index);
                        }
                    };
                }
            }
        }

        if (auto* blendNode = dynamic_cast<Core::Animations::AnimGraphBlendNode*>(node.get()))
        {
            float alpha = 0.5f;

            if (auto* property = blendNode->getProperty("alpha"))
            {
                alpha = std::get<float>(*property);
            }

            ImGui::SetNextItemWidth(140.f);
            if (ImGui::SliderFloat("Alpha", &alpha, 0.0f, 1.0f))
            {
                blendNode->setProperty("alpha", alpha);
            }
        }

        if (auto* IKNode = dynamic_cast<Core::Animations::AnimGraphIKNode*>(node.get()))
        {
            bool signalFromNode = false;
            int targetPickerSolverIndex = -1;

            Core::UI::AnimationInspectorInverseKinematicsUIWindow::renderBody(
                IKNode->getSolvers(), mCurrentMeshComponent, signalFromNode, targetPickerSolverIndex);

            if (signalFromNode)
            {
                mIKSolverPopupOpenRequest = true;
                mIKSolverPopupRequestNode = uuid;
            }

            if (targetPickerSolverIndex != -1)
            {
                mIKTargetPickerOpenRequest = true;
                mIKTargetPickerRequestNode = uuid;
                mIKTargetPickerRequestSolverIndex = targetPickerSolverIndex;
            }
        }

        ImGui::Spacing();

        ImGui::BeginGroup();

        for (const auto& inputPin : editorData.InputPins)
        {
            ed::BeginPin(inputPin.EditorId, ed::PinKind::Input);
            ImGui::Text("In");
            ed::EndPin();
        }

        ImGui::SameLine(160);

        for (const auto& outputPin : editorData.OutputPins)
        {
            ed::BeginPin(outputPin.EditorId, ed::PinKind::Output);
            ImGui::Text("Out");
            ed::EndPin();
        }

        ImGui::EndGroup();

        ImGui::PopItemWidth();

        ImGui::EndGroup();

        ImGui::PopID();

        ed::EndNode();
    }

    for (const auto& link : mEditorLinks)
    {
        ed::Link(link.linkId, link.StartPinId, link.EndPinId);
    }

    if (ed::BeginCreate())
    {
        ed::PinId startPinId;
        ed::PinId endPinId;

        if (ed::QueryNewLink(&startPinId, &endPinId))
        {
            if (startPinId && endPinId)
            {
                const auto* startNodeUuid = findNodeByPin(startPinId.Get());
                const auto* endNodeUuid = findNodeByPin(endPinId.Get());

                if (!startNodeUuid || !endNodeUuid)
                {
                    ed::RejectNewItem();
                }
                else
                {
                    const auto& startNode = mEditorNodes[*startNodeUuid];
                    const auto& endNode = mEditorNodes[*endNodeUuid];

                    const bool startIsInput = containsPin(startNode.InputPins, startPinId.Get());
                    const bool endIsInput = containsPin(endNode.InputPins, endPinId.Get());

                    if (startIsInput == endIsInput)
                    {
                        ed::RejectNewItem(ImColor(255, 0, 0), 2.f);
                    }
                    else
                    {
                        Core::Animations::PinID inputEditorPin;
                        Core::Animations::PinID outputEditorPin;

                        if (startIsInput)
                        {
                            inputEditorPin = startPinId.Get();
                            outputEditorPin = endPinId.Get();
                        }
                        else
                        {
                            inputEditorPin = endPinId.Get();
                            outputEditorPin = startPinId.Get();
                        }

                        if (ed::AcceptNewItem())
                        {
                            removeLinksConnectedToPin(inputEditorPin, animGraph.get());

                            Core::Animations::AnimGraphLink runtimeLink{};

                            runtimeLink.id = generateEditorId();
                            runtimeLink.startPin = findRuntimePin(outputEditorPin);
                            runtimeLink.endPin = findRuntimePin(inputEditorPin);

                            animGraph->addLink(runtimeLink);

                            EditorLinkData editorLink{};
                            editorLink.linkId = runtimeLink.id;
                            editorLink.StartPinId = outputEditorPin;
                            editorLink.EndPinId = inputEditorPin;

                            mEditorLinks.push_back(editorLink);
                        }
                    }
                }
            }
        }
    }

    ed::EndCreate();

    UI::NodeEditorStyle::pop();

    ed::End();

    ImGui::End();

    if (mClipSelectorPopupOpen)
    {
        ImGui::OpenPopup("ClipSelector");
        mClipSelectorPopupOpen = false;
    }

    if (ImGui::BeginPopup(mClipSelectorPopup.id.c_str()))
    {
        for (int i = 0; i < static_cast<int>(mClipSelectorPopup.items.size()); i++)
        {
            if (ImGui::Selectable(mClipSelectorPopup.items[i].c_str()))
            {
                if (mClipSelectorPopup.onSelect)
                {
                    mClipSelectorPopup.onSelect(i);
                }

                ImGui::CloseCurrentPopup();
            }
        }

        ImGui::EndPopup();
    }

    if (mIKSolverPopupOpenRequest)
    {
        ImGui::OpenPopup("CreateIKSolverPopup");
        mIKSolverPopupOpenRequest = false;
    }

    if (const auto targetNode = animGraph->getNode(mIKSolverPopupRequestNode))
    {
        if (auto* IKNode = dynamic_cast<Core::Animations::AnimGraphIKNode*>(targetNode))
        {
            Core::UI::AnimationInspectorInverseKinematicsUIWindow::drawModal(IKNode->getSolvers(),
                                                                             mCurrentMeshComponent);
        }
    }

    if (mIKTargetPickerOpenRequest)
    {
        ImGui::OpenPopup("IKNodeTargetDropdownPopup");
        mIKTargetPickerOpenRequest = false;
    }

    if (const auto targetNode = animGraph->getNode(mIKTargetPickerRequestNode))
    {
        if (auto* IKNode = dynamic_cast<Core::Animations::AnimGraphIKNode*>(targetNode))
        {
            if (const auto& solvers = IKNode->getSolvers();
                mIKTargetPickerRequestSolverIndex >= 0 && mIKTargetPickerRequestSolverIndex < solvers.size())
            {
                auto& solver = solvers[mIKTargetPickerRequestSolverIndex];

                Core::UI::ComponentPicker::RenderOnlyPopup<Core::Component::IKTargetComponent>(
                    "IKNodeTargetDropdownPopup", solver->getTargetUUID(),
                    Core::Engine::getInstance().getSystem<Core::Scene::Scene>(),
                    [&](const uuids::uuid& selectedUUID) { solver->setTargetUUID(selectedUUID); });
            }
        }
    }

    ed::SetCurrentEditor(nullptr);
}

void Editor::Animations::AnimGraphEditorWindow::createEditorNode(const uuids::uuid& uuid)
{
    if (mEditorNodes.contains(uuid))
    {
        return;
    }

    const auto animGraph = mCurrentMeshComponent->getAnimGraph();

    const auto* node = animGraph->getNode(uuid);

    if (!node)
    {
        return;
    }

    EditorNodeData data{};

    data.NodeId = generateEditorId();

    if (const auto* clipNode = dynamic_cast<const Core::Animations::AnimGraphClipNode*>(node))
    {
        data.OutputPins.push_back({.EditorId = generateEditorId(), .RuntimePinId = clipNode->getOutputPin()});
    }
    else if (const auto* blendNode = dynamic_cast<const Core::Animations::AnimGraphBlendNode*>(node))
    {
        data.InputPins.push_back({.EditorId = generateEditorId(), .RuntimePinId = blendNode->getInputAPin()});
        data.InputPins.push_back({.EditorId = generateEditorId(), .RuntimePinId = blendNode->getInputBPin()});
        data.OutputPins.push_back({.EditorId = generateEditorId(), .RuntimePinId = blendNode->getOutputPin()});
    }
    else if (const auto* IKNode = dynamic_cast<const Core::Animations::AnimGraphIKNode*>(node))
    {
        data.InputPins.push_back({.EditorId = generateEditorId(), .RuntimePinId = IKNode->getInputPin()});
        data.OutputPins.push_back({.EditorId = generateEditorId(), .RuntimePinId = IKNode->getOutputPin()});
    }
    else if (const auto* outputNode = dynamic_cast<const Core::Animations::AnimGraphOutputPoseNode*>(node))
    {
        data.InputPins.push_back({.EditorId = generateEditorId(), .RuntimePinId = outputNode->getInputPin()});
    }

    mEditorNodes[uuid] = data;
}

const uuids::uuid* Editor::Animations::AnimGraphEditorWindow::findNodeByPin(const Core::Animations::PinID editorPinId)
{
    for (const auto& [uuid, data] : mEditorNodes)
    {
        if (containsPin(data.InputPins, editorPinId) || containsPin(data.OutputPins, editorPinId))
        {
            return &uuid;
        }
    }

    return nullptr;
}

Core::Animations::PinID
Editor::Animations::AnimGraphEditorWindow::findRuntimePin(const Core::Animations::PinID editorPinId)
{
    for (const auto& [uuid, node] : mEditorNodes)
    {
        for (const auto& pin : node.InputPins)
        {
            if (pin.EditorId == editorPinId)
            {
                return pin.RuntimePinId;
            }
        }

        for (const auto& pin : node.OutputPins)
        {
            if (pin.EditorId == editorPinId)
            {
                return pin.RuntimePinId;
            }
        }
    }

    return {};
}

bool Editor::Animations::AnimGraphEditorWindow::containsPin(const std::vector<EditorPinData>& pins,
                                                            const uint64_t editorPinId)
{
    return std::ranges::any_of(pins, [editorPinId](const EditorPinData& pin) { return pin.EditorId == editorPinId; });
}

void Editor::Animations::AnimGraphEditorWindow::removeLinksConnectedToPin(Core::Animations::PinID editorPinId,
                                                                          Core::Animations::AnimGraph* animGraph)
{
    std::erase_if(mEditorLinks, [editorPinId](const EditorLinkData& link)
                  { return link.StartPinId == editorPinId || link.EndPinId == editorPinId; });

    animGraph->removeLinksByPin(findRuntimePin(editorPinId));
}

uint64_t Editor::Animations::AnimGraphEditorWindow::generateEditorId() { return mNextEditorId++; }