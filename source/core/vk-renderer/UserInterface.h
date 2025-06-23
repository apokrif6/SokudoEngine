#pragma once

#include "VkRenderData.h"

namespace Core::Renderer
{
class UserInterface
{
  public:
    bool init(VkRenderData& renderData);

    void createFrame(VkRenderData& renderData);

    void render(VkRenderData& renderData);

    void cleanup(VkRenderData& renderData);

  private:
    void setupImGuiStyle() const;

    float mFramesPerSecond = 0.0f;
    float mAveragingAlpha = 0.95f;
};
} // namespace Core::Renderer