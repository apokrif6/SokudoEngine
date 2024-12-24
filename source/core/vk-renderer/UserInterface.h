#pragma once

#include "VkRenderData.h"

namespace Core::Renderer
{
class UserInterface
{
  public:
    bool init(Core::Renderer::VkRenderData& renderData);

    void createFrame(Core::Renderer::VkRenderData& renderData);

    void render(Core::Renderer::VkRenderData& renderData);

    void cleanup(Core::Renderer::VkRenderData& renderData);

  private:
    float mFramesPerSecond = 0.0f;
    float mAveragingAlpha = 0.95f;
};
}