#pragma once

#include "VkRenderData.h"

class SyncObjects
{
  public:
    static bool init(VkRenderData& renderData);

    static void cleanup(VkRenderData& renderData);
};