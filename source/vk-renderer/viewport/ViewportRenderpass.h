#pragma once

#include "vk-renderer/VkRenderData.h"

namespace Core::Renderer
{
class ViewportRenderpass
{
public:
    static bool init(VkRenderData& renderData);

    static void cleanup(VkRenderData& renderData);
};
} // namespace Core::Renderer