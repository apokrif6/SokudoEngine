#pragma once

#include <string>

#include "core/components/Component.h"
#include "core/vk-renderer/VkRenderData.h"
#include "yaml-cpp/node/node.h"
#include "core/serialization/Serializable.h"

namespace Core::Scene
{
class SceneObject final : public Serialization::ISerializable
{
public:
    explicit SceneObject(std::string name) : mName(std::move(name)) {}
    virtual ~SceneObject() = default;

    template<typename T, typename... Args>
        T* addComponent(Args&&... args)
    {
        static_assert(std::is_base_of_v<Component::Component, T>);

        auto component = std::make_unique<T>(std::forward<Args>(args)...);
        T* ptr = component.get();

        ptr->setOwner(this);
        ptr->onAdded();

        mComponents.emplace_back(std::move(component));
        return ptr;
    }

    template<typename T>
    T* getComponent() const
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

protected:
    std::string mName;
    SceneObject* mParent = nullptr;
    std::vector<std::shared_ptr<SceneObject>> mChildren;
    std::vector<std::unique_ptr<Component::Component>> mComponents;
};
} // namespace Core::Scene