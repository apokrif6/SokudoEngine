#pragma once

#include "VkRenderData.h"

namespace Core::Renderer
{
class CommandPool
{
public:
    static bool init(VkRenderData& renderData);

    static void cleanup(const VkRenderData& renderData);
};
} // namespace Core::Renderer