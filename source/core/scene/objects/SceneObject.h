#pragma once

#include <string>
#include "core/scene/Transform.h"
#include "core/vk-renderer/VkRenderData.h"
#include "yaml-cpp/node/node.h"
#include "core/serialization/Serializable.h"

namespace Core::Scene
{
enum class ObjectType
{
    Empty,
    Mesh
};

class SceneObject : public Serialization::ISerializable
{
public:
    explicit SceneObject(std::string name) : mName(std::move(name)) {}
    virtual ~SceneObject() = default;

    [[nodiscard]] virtual Scene::ObjectType getType() const { return ObjectType::Empty; }

    virtual void onAddedToScene() {}

    virtual void onRemovedFromScene() {}

    virtual void update(Renderer::VkRenderData& renderData) {};

    virtual void draw(Renderer::VkRenderData& renderData) {};

    virtual void cleanup(Renderer::VkRenderData& renderData) {};

    Transform& getTransform() { return mTransform; }

    [[nodiscard]] SceneObject* getParent() const { return mParent; }

    [[nodiscard]] std::vector<std::shared_ptr<SceneObject>> getChildren() const { return mChildren; }

    [[nodiscard]] const std::string& getName() const { return mName; }

    [[nodiscard]] YAML::Node serialize() const override;

    void deserialize(const YAML::Node& node) override;

    void addChild(const std::shared_ptr<SceneObject>& child);

    void removeChild(SceneObject* child);

protected:
    std::string mName;
    Transform mTransform;
    SceneObject* mParent = nullptr;
    std::vector<std::shared_ptr<SceneObject>> mChildren;
};
} // namespace Core::Scene