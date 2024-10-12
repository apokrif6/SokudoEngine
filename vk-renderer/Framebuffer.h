#pragma once

#include "VkRenderData.h"

class Framebuffer
{
  public:
    static bool init(VkRenderData& renderData);
    static void cleanup(VkRenderData& renderData);
};
