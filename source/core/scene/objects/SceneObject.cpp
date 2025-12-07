#include "SceneObject.h"
#include "yaml-cpp/node/node.h"

YAML::Node Core::Scene::SceneObject::serialize() const
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
}

void Core::Scene::SceneObject::deserialize(const YAML::Node& node)
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
};

void Core::Scene::SceneObject::addChild(const std::shared_ptr<SceneObject>& child)
{
    child->mParent = this;
    mChildren.push_back(child);
}

void Core::Scene::SceneObject::removeChild(SceneObject* child)
{
    mChildren.erase(std::remove_if(mChildren.begin(), mChildren.end(),
                           [&](const auto& c){ return c.get() == child; }),
            mChildren.end());
}