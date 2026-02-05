#pragma once

#include "imgui.h"
#include "cmath"

namespace Core::Tools
{
class ColorConverter
{
public:
    static float sRGBToLinear(const float color)
    {
        if (color <= 0.04045f)
        {
            return color / 12.92f;
        }
        return std::pow((color + 0.055f) / 1.055f, 2.4f);
    }

    static ImVec4 toLinear(float r, float g, float b, float a = 1.0f)
    {
        return {sRGBToLinear(r), sRGBToLinear(g), sRGBToLinear(b), a};
    }

    static ImVec4 toLinear(const ImVec4& color)
    {
        return {sRGBToLinear(color.x), sRGBToLinear(color.y), sRGBToLinear(color.z), color.w};
    }
};
} // namespace Core::Tools