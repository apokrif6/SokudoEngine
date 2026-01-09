#pragma once

#include "core/serialization/Serializable.h"
#include "core/vk-renderer/VkRenderData.h"

namespace Core::Scene {
    class SceneObject;
}

namespace Core::Component
{
class Component : public Serialization::ISerializable
{
public:
    virtual ~Component() = default;

    void setOwner(Scene::SceneObject* owner) { mOwner = owner; }
    Scene::SceneObject* getOwner() const { return mOwner; }

    [[nodiscard]] virtual std::string_view getTypeName() const = 0;

    virtual void onAdded() {}
    virtual void onRemoved() {}

    virtual void onAddedToScene() {}
    virtual void onRemovedFromScene() {}

    virtual void update(Renderer::VkRenderData& renderData) {}
    virtual void draw(Renderer::VkRenderData& renderData) {}
    virtual void cleanup(Renderer::VkRenderData& renderData) {}

protected:
    Scene::SceneObject* mOwner = nullptr;
};
}
