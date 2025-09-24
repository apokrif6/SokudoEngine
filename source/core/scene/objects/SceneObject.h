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

    virtual void update(Renderer::VkRenderData& renderData) {};

    virtual void draw(Renderer::VkRenderData& renderData) {};

    virtual void cleanup(Renderer::VkRenderData& renderData) {};

    Transform& getTransform() { return mTransform; }

    [[nodiscard]] const Transform& getTransform() const { return mTransform; }

    [[nodiscard]] const std::string& getName() const { return mName; }

    YAML::Node serialize() const override
    {
        YAML::Node node;
        node["type"] = static_cast<int>(getType());
        node["name"] = mName;

        YAML::Node position;
        position.push_back(mTransform.position.x);
        position.push_back(mTransform.position.y);
        position.push_back(mTransform.position.z);
        node["transform"]["position"] = position;

        YAML::Node rotation;
        rotation.push_back(mTransform.rotation.w);
        rotation.push_back(mTransform.rotation.x);
        rotation.push_back(mTransform.rotation.y);
        rotation.push_back(mTransform.rotation.z);
        node["transform"]["rotation"] = rotation;

        YAML::Node scale;
        scale.push_back(mTransform.scale.x);
        scale.push_back(mTransform.scale.y);
        scale.push_back(mTransform.scale.z);
        node["transform"]["scale"] = scale;

        return node;
    };

    void deserialize(const YAML::Node& node) override
    {
        mName = node["name"].as<std::string>();

        auto transformNode = node["transform"];
        mTransform.position = glm::vec3(
                transformNode["position"][0].as<float>(),
                transformNode["position"][1].as<float>(),
                transformNode["position"][2].as<float>()
        );

        mTransform.rotation = glm::quat(
                transformNode["rotation"][0].as<float>(),
                transformNode["rotation"][1].as<float>(),
                transformNode["rotation"][2].as<float>(),
                transformNode["rotation"][3].as<float>()
        );

        mTransform.scale = glm::vec3(
                transformNode["scale"][0].as<float>(),
                transformNode["scale"][1].as<float>(),
                transformNode["scale"][2].as<float>()
        );
    }

  protected:
    std::string mName;
    Transform mTransform;
};
} // namespace Core::Scene