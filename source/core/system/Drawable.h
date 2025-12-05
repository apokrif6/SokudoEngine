#pragma once

#include "core/vk-renderer/VkRenderData.h"

namespace Core::System
{
enum class DrawLayer
{
    World,
    Overlay
};

class IDrawable {
public:
    virtual ~IDrawable() = default;

    [[nodiscard]] virtual DrawLayer getDrawLayer() const = 0;

    virtual void draw(Renderer::VkRenderData& renderData) = 0;
};
}