#include "PlotBuffer.h"
#include "algorithm"
#include "imgui.h"
#include "imgui_internal.h"
#include "numeric"
#include "string"

#include <format>

void Core::Profiling::PlotBuffer::push(const float value)
{
    if (values.size() >= maxSize)
    {
        values.pop_front();
    }
    values.push_back(value);
}

void Core::Profiling::PlotBuffer::draw(const char* label)
{

    if (values.empty())
    {
        return;
    }

    const float minValue = *std::ranges::min_element(values);
    const float maxValue = *std::ranges::max_element(values);
    const float averageValue = std::accumulate(values.begin(), values.end(), 0.f) / static_cast<float>(values.size());
    const float lastValue = values.back();

    ImGui::Separator();
    ImGui::Text("%s", label);
    ImGui::PlotLines(
        std::format("##{}", label).c_str(),
        [](void* data, int idx)
        {
            const auto* value = static_cast<std::deque<float>*>(data);
            return (*value)[idx];
        },
        &values, static_cast<int>(values.size()), 0, nullptr, 0.f, maxValue, ImVec2(0, 60));

    const ImRect infoRect(ImGui::GetItemRectMin(), ImGui::GetItemRectMax());
    ImDrawList* drawList = ImGui::GetWindowDrawList();

    for (float value = 0.f; value <= maxValue; value += 1.f)
    {
        float py = ImLerp(infoRect.Max.y, infoRect.Min.y, value / maxValue);
        drawList->AddLine(ImVec2(infoRect.Min.x, py), ImVec2(infoRect.Max.x, py), IM_COL32(255, 255, 0, 100));
        drawList->AddText(ImVec2(infoRect.Min.x, py - 6), IM_COL32(255, 255, 255, 200),
                          (std::to_string(static_cast<int>(value)) + " ms").c_str());
    }

    ImGui::SetCursorScreenPos(ImVec2(infoRect.Max.x + 10, infoRect.Min.y));
    ImGui::BeginGroup();
    ImGui::Text("Min:  %.2f ms", minValue);
    ImGui::Text("Max:  %.2f ms", maxValue);
    ImGui::Text("Avg:  %.2f ms", averageValue);
    ImGui::Text("Last: %.2f ms", lastValue);
    ImGui::EndGroup();
}
