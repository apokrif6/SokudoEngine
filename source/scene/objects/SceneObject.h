#pragma once

#include <string>

#include "components/Component.h"
#include "vk-renderer/VkRenderData.h"
#include "yaml-cpp/node/node.h"
#include "serialization/Serializable.h"
#include <random>

namespace Core::Scene
{
class SceneObject final : public Serialization::ISerializable
{
public:
    explicit SceneObject(std::string name) : mName(std::move(name)), mUUID(generateUUID()) {}
    ~SceneObject() override = default;

    [[nodiscard]] uint64_t getUUID() const { return mUUID; }

    void setUUID(const uint64_t uuid) { mUUID = uuid; }

    template <typename T, typename... Args> T* addComponent(Args&&... args)
    {
        static_assert(std::is_base_of_v<Component::Component, T>);

        auto component = std::make_unique<T>(std::forward<Args>(args)...);
        T* ptr = component.get();

        ptr->setOwner(this);
        ptr->onAdded();

        mComponents.emplace_back(std::move(component));
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
    SceneObject* mParent = nullptr;
    std::vector<std::shared_ptr<SceneObject>> mChildren;
    std::vector<std::unique_ptr<Component::Component>> mComponents;

private:
    uint64_t mUUID;

    // TODO
    // use stduuid
    static uint64_t generateUUID()
    {
        static std::random_device randomDevice;
        static std::mt19937_64 generator(randomDevice());
        static std::uniform_int_distribution<uint64_t> distribution;
        return distribution(generator);
    }
};
} // namespace Core::Scene