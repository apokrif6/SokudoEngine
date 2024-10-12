#pragma once

#include "VkRenderData.h"

class CommandPool
{
  public:
    static bool init(VkRenderData& renderData);
    static void cleanup(VkRenderData& renderData);
};
