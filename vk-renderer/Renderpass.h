#pragma once

#include "VkRenderData.h"

class Renderpass
{
  public:
    static bool init(VkRenderData& renderData);
    static void cleanup(VkRenderData& renderData);
};
