#pragma once

#include "core/IUUIDObject.h"
#include <string>
#include "components/Component.h"
#include "vk-renderer/VkRenderData.h"
#include "yaml-cpp/node/node.h"
#include "serialization/Serializable.h"
#include <random>
#include "uuid.h"

namespace Core::Scene
{
class Scene;

class SceneObject final : public Serialization::ISerializable, public IUUIDObject
{
public:
    explicit SceneObject(std::string name, Scene* scene)
        : mName(std::move(name)), mScene(scene), mUUID(UUID::generateUUID())
    {
    }
    ~SceneObject() override = default;

    [[nodiscard]] const uuids::uuid& getUUID() const override { return mUUID; }

    void setUUID(const uuids::uuid uuid) { mUUID = uuid; }

    template <typename T, typename... Args> T* addComponent(Args&&... args)
    {
        static_assert(std::is_base_of_v<Component::Component, T>);

        auto component = std::make_unique<T>(std::forward<Args>(args)...);
        T* ptr = component.get();

        addComponentInternal(std::move(component));

        return ptr;
    }

    template <typename T> T* getComponent() const
    {
        for (auto& component : mComponents)
        {
            if (auto castedComponent = dynamic_cast<T*>(component.get()))
            {
                return castedComponent;
            }
        }
        return nullptr;
    }

    [[nodiscard]]
    const std::vector<std::unique_ptr<Component::Component>>& getComponents() const
    {
        return mComponents;
    }

    void update(Renderer::VkRenderData& renderData);

    void draw(Renderer::VkRenderData& renderData);

    void cleanup(Renderer::VkRenderData& renderData);

    [[nodiscard]] SceneObject* getParent() const { return mParent; }

    [[nodiscard]] std::vector<std::shared_ptr<SceneObject>> getChildren() const { return mChildren; }

    [[nodiscard]] const std::string& getName() const { return mName; }

    [[nodiscard]] YAML::Node serialize() const override;

    void deserialize(const YAML::Node& node) override;

    void addChild(const std::shared_ptr<SceneObject>& child);

    void removeChild(SceneObject* child);

    void setName(const std::string_view& name) { mName = name; }

protected:
    std::string mName;
    Scene* mScene = nullptr;
    SceneObject* mParent = nullptr;
    std::vector<std::shared_ptr<SceneObject>> mChildren;
    std::vector<std::unique_ptr<Component::Component>> mComponents;

private:
    uuids::uuid mUUID;

    void addComponentInternal(std::unique_ptr<Component::Component> component);

    void registerComponentInternal(Component::Component* component) const;
};
} // namespace Core::Scene