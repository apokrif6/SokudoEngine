#pragma once

#include "core/IUUIDObject.h"
#include "serialization/Serializable.h"
#include "vk-renderer/VkRenderData.h"
#include "uuid.h"

namespace Core::Scene
{
class SceneObject;
}

namespace Core::Component
{
class Component : public Serialization::ISerializable, public IUUIDObject
{
public:
    Component() : mUUID(UUID::generateUUID()) {}
    ~Component() override = default;

    [[nodiscard]] const uuids::uuid& getUUID() const override { return mUUID; }

    void setOwner(Scene::SceneObject* owner) { mOwner = owner; }
    [[nodiscard]] Scene::SceneObject* getOwner() const { return mOwner; }

    [[nodiscard]] virtual std::string_view getTypeName() const = 0;

    virtual void onAdded() {}
    virtual void onRemoved() {}

    virtual void update(Renderer::VkRenderData& renderData) {}
    virtual void draw(Renderer::VkRenderData& renderData) {}
    virtual void cleanup(Renderer::VkRenderData& renderData) {}

protected:
    uuids::uuid mUUID;
    Scene::SceneObject* mOwner = nullptr;
};
} // namespace Core::Component
