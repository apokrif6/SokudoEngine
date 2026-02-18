#include "SceneObject.h"
#include "yaml-cpp/node/node.h"
#include <algorithm>
#include "components/ComponentFactory.h"
#include "components/TransformComponent.h"

void Core::Scene::SceneObject::update(Renderer::VkRenderData& renderData)
{
    for (auto& component : mComponents)
    {
        component->update(renderData);
    }

    for (auto& child : mChildren)
    {
        child->update(renderData);
    }
}

void Core::Scene::SceneObject::draw(Renderer::VkRenderData& renderData)
{
    for (auto& component : mComponents)
    {
        component->draw(renderData);
    }

    for (auto& child : mChildren)
    {
        child->draw(renderData);
    }
}

void Core::Scene::SceneObject::cleanup(Renderer::VkRenderData& renderData)
{
    for (auto& component : mComponents)
    {
        component->onRemoved();
        component->cleanup(renderData);
    }

    for (auto& child : mChildren)
    {
        child->cleanup(renderData);
    }
}

YAML::Node Core::Scene::SceneObject::serialize() const
{
    YAML::Node node;
    node["name"] = mName;

    YAML::Node componentsNode;

    for (const auto& component : mComponents)
    {
        if (auto serializable = dynamic_cast<ISerializable*>(component.get()))
        {
            YAML::Node compNode;
            compNode["type"] = component->getTypeName().data();
            compNode["data"] = serializable->serialize();
            componentsNode.push_back(compNode);
        }
    }

    node["components"] = componentsNode;

    for (const auto& child : mChildren)
    {
        node["children"].push_back(child->serialize());
    }

    return node;
}

void Core::Scene::SceneObject::deserialize(const YAML::Node& node)
{
    mName = node["name"].as<std::string>();

    if (const auto componentsNode = node["components"])
    {
        for (const auto& componentNode : componentsNode)
        {
            const std::string type = componentNode["type"].as<std::string>();

            auto component = Component::ComponentFactory::create(type);
            component->setOwner(this);
            component->onAdded();
            component->deserialize(componentNode["data"]);

            mComponents.push_back(std::move(component));
        }
    }

    if (const auto childrenNode = node["children"])
    {
        for (const auto& childNode : childrenNode)
        {
            auto child = std::make_shared<SceneObject>("");
            child->deserialize(childNode);
            addChild(child);
        }
    }
}

void Core::Scene::SceneObject::addChild(const std::shared_ptr<SceneObject>& child)
{
    child->mParent = this;
    mChildren.push_back(child);
}

void Core::Scene::SceneObject::removeChild(SceneObject* child)
{
    mChildren.erase(std::remove_if(mChildren.begin(), mChildren.end(), [&](const auto& c) { return c.get() == child; }),
                    mChildren.end());
}