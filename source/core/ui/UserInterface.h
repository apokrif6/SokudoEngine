#pragma once

#include "core/vk-renderer/VkRenderData.h"

namespace Core::Renderer
{
class UserInterface
{
  public:
    bool init(VkRenderData& renderData);

    void update(VkRenderData& renderData);

    void draw(VkRenderData& renderData);

    void cleanup(VkRenderData& renderData);

  private:
    void setupImGuiStyle() const;


};
} // namespace Core::Renderer