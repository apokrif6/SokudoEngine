#include "AnimGraphEditorWindow.h"

#include "imgui.h"
#include "animations/anim-graph/nodes/AnimGraphBlendNode.h"
#include "animations/anim-graph/nodes/AnimGraphClipNode.h"
#include "animations/anim-graph/nodes/AnimGraphOutputPoseNode.h"
#include "components/MeshComponent.h"

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

    if (ImGui::Button("Add Clip Node"))
    {
        const auto node = animGraph->createNode<Core::Animations::AnimGraphClipNode>();

        node->setProperty("clipIndex", 0);

        CreateEditorNode(node->getUUID());
    }

    ImGui::SameLine();

    if (ImGui::Button("Add Blend Node"))
    {
        const auto node = animGraph->createNode<Core::Animations::AnimGraphBlendNode>();

        node->setProperty("alpha", 0.5f);

        CreateEditorNode(node->getUUID());
    }

    ImGui::Separator();

    ed::Begin("AnimGraph");

    for (auto& [uuid, node] : animGraph->getNodes())
    {
        if (!node)
        {
            continue;
        }

        if (!mEditorNodes.contains(uuid))
        {
            CreateEditorNode(uuid);
        }

        const auto& editorData = mEditorNodes[uuid];

        ed::BeginNode(editorData.NodeId);

        ed::BeginPin(editorData.InputPinId, ed::PinKind::Input);
        ImGui::Text("-> In");
        ed::EndPin();

        ImGui::Spacing();

        ImGui::Text("Node");
        ImGui::TextWrapped("%s", uuids::to_string(uuid).c_str());

        ImGui::Separator();

        if (const auto* clipNode = dynamic_cast<Core::Animations::AnimGraphClipNode*>(node.get()))
        {
            ImGui::TextColored(ImVec4(0, 1, 0, 1), "Clip Node");
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
            ImGui::TextColored(ImVec4(0, 0.7f, 1, 1), "Blend Node");

            float alpha = 0.5f;

            if (auto* property = blendNode->getProperty("alpha"))
            {
                alpha = std::get<float>(*property);
            }

            if (ImGui::SliderFloat("Alpha", &alpha, 0.0f, 1.0f))
            {
                blendNode->setProperty("alpha", alpha);
            }
        }

        if (auto* outputPoseNode = dynamic_cast<Core::Animations::AnimGraphOutputPoseNode*>(node.get()))
        {
            ImGui::TextColored(ImVec4(0.6f, 0.f, 1.f, 1.f), "Output Pose Node");
        }

        ImGui::Separator();

        ed::BeginPin(editorData.OutputPinId, ed::PinKind::Output);
        ImGui::Text("Out ->");
        ed::EndPin();

        ed::EndNode();
    }

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

    ed::SetCurrentEditor(nullptr);
}

void Editor::Animations::AnimGraphEditorWindow::CreateEditorNode(const uuids::uuid& uuid)
{
    if (mEditorNodes.contains(uuid))
    {
        return;
    }

    EditorNodeData data{};
    data.NodeId = mNextId++;
    data.InputPinId = mNextId++;
    data.OutputPinId = mNextId++;

    mEditorNodes[uuid] = data;
}
