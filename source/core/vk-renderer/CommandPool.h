#pragma once

#include "VkRenderData.h"

namespace Core::Renderer
{
class CommandPool
{
  public:
    static bool init(Core::Renderer::VkRenderData& renderData);

    static void cleanup(const Core::Renderer::VkRenderData& renderData);
};
}