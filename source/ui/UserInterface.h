#pragma once

#include "core/vk-renderer/VkRenderData.h"
#include "core/events/EventListener.h"
#include "core/events/EventDispatcher.h"
#include "core/system/System.h"
#include "core/system/Updatable.h"
#include "core/system/Drawable.h"

namespace Core::Renderer
{
class UserInterface : public EventListener, public System::ISystem, public System::IUpdatable, public System::IDrawable
{
public:
    bool init(VkRenderData& renderData);

    virtual void update(VkRenderData& renderData, float deltaTime) override;

    virtual System::DrawLayer getDrawLayer() const override { return System::DrawLayer::Overlay; }

    virtual void draw(VkRenderData& renderData) override;

    void cleanup(VkRenderData& renderData);

    void onEvent(const Event& event) override;

private:
    void setupImGuiStyle() const;
};
} // namespace Core::Renderer