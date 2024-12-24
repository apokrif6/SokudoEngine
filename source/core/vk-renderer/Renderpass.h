#pragma once

#include "VkRenderData.h"

namespace Core::Renderer
{
class Renderpass
{
  public:
    static bool init(Core::Renderer::VkRenderData renderData);

    static void cleanup(Core::Renderer::VkRenderData renderData);
};
}