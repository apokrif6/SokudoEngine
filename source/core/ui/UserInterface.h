#pragma once

#include "core/vk-renderer/VkRenderData.h"
#include "core/events/EventListener.h"
#include "core/events/EventDispatcher.h"

namespace Core::Renderer
{
class UserInterface : public EventListener 
{
  public:
    bool init(VkRenderData& renderData);

    void update(VkRenderData& renderData);

    void draw(VkRenderData& renderData);

    void cleanup(VkRenderData& renderData);

    void onEvent(const Event& event) override;

  private:
    void setupImGuiStyle() const;
};
} // namespace Core::Renderer