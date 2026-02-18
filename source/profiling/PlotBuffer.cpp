#include "PlotBuffer.h"
#include "algorithm"
#include "imgui.h"
#include "imgui_internal.h"
#include "numeric"
#include <format>

Core::Profiling::PlotBuffer::PlotBuffer(const size_t maxSize) : offset(0) { values.resize(maxSize, 0.0f); }

void Core::Profiling::PlotBuffer::push(const float value)
{
    values[offset] = value;
    offset = (offset + 1) % values.size();

    updateStats();
}

void Core::Profiling::PlotBuffer::updateStats()
{
    float minVal = std::numeric_limits<float>::max();
    float maxVal = std::numeric_limits<float>::lowest();
    float sum = 0.0f;

    for (float value : values)
    {
        minVal = std::min(value, minVal);
        maxVal = std::max(value, maxVal);
        sum += value;
    }

    stats.min = minVal;
    stats.max = maxVal;
    stats.average = sum / static_cast<float>(values.size());

    const size_t lastIdx = offset == 0 ? values.size() - 1 : offset - 1;
    stats.last = values[lastIdx];
}

void Core::Profiling::PlotBuffer::draw(const char* label)
{
    ImGui::PushID(label);

    ImGui::Separator();
    ImGui::Text("%s", label);
    ImGui::PlotLines("##PlotBufferLines", values.data(), static_cast<int>(values.size()), static_cast<int>(offset),
                     nullptr, 0.f, stats.max * 1.1f, ImVec2(0, 60));

    const ImRect infoRect(ImGui::GetItemRectMin(), ImGui::GetItemRectMax());
    ImDrawList* drawList = ImGui::GetWindowDrawList();

    const float maxVal = stats.max * 1.1f;

    if (maxVal > 0.0f)
    {
        constexpr int numGridLines = 5;

        for (int i = 1; i < numGridLines; ++i)
        {
            const float yRatio = static_cast<float>(i) / static_cast<float>(numGridLines);
            const float valAtLine = maxVal * yRatio;

            const float py = ImLerp(infoRect.Max.y, infoRect.Min.y, yRatio);

            drawList->AddLine(ImVec2(infoRect.Min.x, py), ImVec2(infoRect.Max.x, py), IM_COL32(255, 255, 0, 50));

            char gridLabel[16];
            const int writtenCharactersNumber = snprintf(gridLabel, sizeof(gridLabel), "%.3f", valAtLine);
            if (writtenCharactersNumber > 0)
            {
                drawList->AddText(ImVec2(infoRect.Min.x + 2, py - 12), IM_COL32(255, 255, 255, 150), gridLabel);
            }
        }
    }
    drawList->AddRect(infoRect.Min, infoRect.Max, IM_COL32(255, 255, 255, 30));

    ImGui::SameLine();
    ImGui::BeginGroup();
    ImGui::Text("Min:  %.2f ms", stats.min);
    ImGui::Text("Max:  %.2f ms", stats.max);
    ImGui::Text("Avg:  %.2f ms", stats.average);
    ImGui::Text("Last: %.2f ms", stats.last);
    ImGui::EndGroup();

    ImGui::PopID();
}
