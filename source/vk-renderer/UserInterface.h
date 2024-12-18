#pragma once

#include "VkRenderData.h"

class UserInterface
{
  public:
    bool init(VkRenderData& renderData);

    void createFrame(VkRenderData& renderData);

    void render(VkRenderData& renderData);

    void cleanup(VkRenderData& renderData);

  private:
    float mFramesPerSecond = 0.0f;
    float mAveragingAlpha = 0.95f;
};
