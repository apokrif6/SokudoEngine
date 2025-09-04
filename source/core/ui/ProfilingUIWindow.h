#pragma once

#include "UIWindow.h"
#include "imgui.h"
#include "imgui_internal.h"
#include "string"
#include "deque"
#include "algorithm"
#include "core/engine/Engine.h"

#include <numeric>

namespace Core::UI
{
struct PlotBuffer
{
    std::deque<float> values;
    size_t maxSize;

    explicit PlotBuffer(const size_t inMaxSize = 60) : maxSize(inMaxSize) {}

    void push(const float value)
    {
        if (values.size() >= maxSize)
        {
            values.pop_front();
        }
        values.push_back(value);
    }

    void draw(const char* label)
    {
        if (values.empty())
        {
            return;
        }

        float minVal = *std::ranges::min_element(values);
        float maxVal = *std::ranges::max_element(values);
        float avgVal = std::accumulate(values.begin(), values.end(), 0.f) / static_cast<float>(values.size());
        float lastVal = values.back();

        ImGui::Text("%s", label);

        ImGui::PlotLines(
            "##AnimationUpdatePlot",
            [](void* data, int idx)
            {
                const auto* value = static_cast<std::deque<float>*>(data);
                return (*value)[idx];
            },
            &values, static_cast<int>(values.size()), 0, nullptr, 0.0f, maxVal, ImVec2(0, 80));

        const ImRect infoRect(ImGui::GetItemRectMin(), ImGui::GetItemRectMax());
        ImDrawList* drawList = ImGui::GetWindowDrawList();

        for (float value = 0.f; value <= maxVal; value += 1.f)
        {
            float py = ImLerp(infoRect.Max.y, infoRect.Min.y, value / maxVal);
            drawList->AddLine(ImVec2(infoRect.Min.x, py), ImVec2(infoRect.Max.x, py), IM_COL32(255, 255, 0, 100));
            drawList->AddText(ImVec2(infoRect.Min.x, py - 6), IM_COL32(255, 255, 255, 200),
                              (std::to_string(static_cast<int>(value)) + " ms").c_str());
        }

        ImGui::SetCursorScreenPos(ImVec2(infoRect.Max.x + 10, infoRect.Min.y));
        ImGui::BeginGroup();
        ImGui::Text("Min:  %.2f ms", minVal);
        ImGui::Text("Max:  %.2f ms", maxVal);
        ImGui::Text("Avg:  %.2f ms", avgVal);
        ImGui::Text("Last: %.2f ms", lastVal);
        ImGui::EndGroup();
    }
};

class ProfilingUIWindow : public UIWindow<ProfilingUIWindow>
{
  public:
    static bool getBody()
    {
        if (!ImGui::BeginTabItem("Profiling"))
        {
            return false;
        }

        Renderer::VkRenderData& renderData = Core::Engine::getInstance().getRenderData();
        static float newFps = 0.0f;
        if (renderData.rdFrameTime > 0.0)
        {
            newFps = 1.0f / renderData.rdFrameTime * 1000.f;
        }

        mFramesPerSecond = (mAveragingAlpha * mFramesPerSecond) + (1.0f - mAveragingAlpha) * newFps;

        ImGui::Text("Frames per second:");
        ImGui::SameLine();
        ImGui::Text("%s", std::to_string(mFramesPerSecond).c_str());

        ImGui::Text("Frame Time:");
        ImGui::SameLine();
        ImGui::Text("%s", std::to_string(renderData.rdFrameTime).c_str());
        ImGui::SameLine();
        ImGui::Text("ms");

        ImGui::Text("Matrix Generation Time:");
        ImGui::SameLine();
        ImGui::Text("%s", std::to_string(renderData.rdMatrixGenerateTime).c_str());
        ImGui::SameLine();
        ImGui::Text("ms");

        ImGui::Text("Matrix Upload Time:");
        ImGui::SameLine();
        ImGui::Text("%s", std::to_string(renderData.rdUploadToUBOTime).c_str());
        ImGui::SameLine();
        ImGui::Text("ms");

        mAnimationPlot.push(renderData.rdAnimationBonesTransformCalculationTime);
        mAnimationPlot.draw("Animation  Update Time");

        ImGui::EndTabItem();

        return true;
    }

  private:
    inline static float mFramesPerSecond = 0.0f;
    inline static float mAveragingAlpha = 0.95f;

    inline static PlotBuffer mAnimationPlot{200};
};
} // namespace Core::UI
