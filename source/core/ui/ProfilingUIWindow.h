#pragma once

#include "UIWindow.h"
#include "imgui.h"
#include "string"
#include "core/engine/Engine.h"
#include "core/profiling/PlotBuffer.h"

namespace Core::UI
{
class ProfilingUIWindow : public UIWindow<ProfilingUIWindow>
{
public:
    static bool getBody()
    {
        if (!ImGui::Begin("Profiling"))
        {
            return false;
        }

        Renderer::VkRenderData& renderData = Engine::getInstance().getRenderData();
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

        mScenePlot.push(renderData.rdUpdateSceneProfilingTime);
        mScenePlot.draw("Scene Update Time");

        mAnimationPlot.push(renderData.rdAnimationBonesTransformCalculationTime);
        mAnimationPlot.draw("Animation Update Time");

        ImGui::End();

        return true;
    }

private:
    inline static float mFramesPerSecond = 0.0f;
    inline static float mAveragingAlpha = 0.95f;

    inline static Profiling::PlotBuffer mScenePlot{200};
    inline static Profiling::PlotBuffer mAnimationPlot{200};
};
} // namespace Core::UI
