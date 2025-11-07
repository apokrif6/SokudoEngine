#pragma once

#include "core/vk-renderer/VkRenderData.h"

namespace Core::Renderer
{
class ViewportRenderpass
{
public:
    static bool init(Core::Renderer::VkRenderData& renderData);

    static void cleanup(Core::Renderer::VkRenderData& renderData);
};
}