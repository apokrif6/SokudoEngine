#pragma once

#include "core/vk-renderer/VkRenderData.h"

namespace Core::System
{
class IUpdatable
{
public:
    virtual ~IUpdatable() = default;

    virtual void update(Renderer::VkRenderData& renderData, float deltaTime) = 0;
};
} // namespace Core::System