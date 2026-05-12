#include "SceneObject.h"
#include "yaml-cpp/node/node.h"
#include <algorithm>
#include "components/ComponentFactory.h"
#include "components/TransformComponent.h"
#include "serialization/UUIDSerializationConverter.h"

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
    mComponents.clear();

    for (auto& child : mChildren)
    {
        child->cleanup(renderData);
    }
    mChildren.clear();
}

YAML::Node Core::Scene::SceneObject::serialize() const
{
    YAML::Node node;
    node["uuid"] = mUUID;
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
    mUUID = node["uuid"].as<uuids::uuid>();
    mName = node["name"].as<std::string>();

    if (const auto componentsNode = node["components"])
    {
        for (const auto& componentNode : componentsNode)
        {
            const auto type = componentNode["type"].as<std::string>();

            auto component = Component::ComponentFactory::create(type);

            component->deserialize(componentNode["data"]);

            addComponentInternal(std::move(component));
        }
    }

    if (const auto childrenNode = node["children"])
    {
        for (const auto& childNode : childrenNode)
        {
            auto child = std::make_shared<SceneObject>("", mScene);
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

void Core::Scene::SceneObject::addComponentInternal(std::unique_ptr<Component::Component> component)
{
    component->setOwner(this);

    Component::Component* ptr = component.get();

    mComponents.emplace_back(std::move(component));

    registerComponentInternal(ptr);

    ptr->onAdded();
}

void Core::Scene::SceneObject::registerComponentInternal(Component::Component* component) const
{
    mScene->registerComponent(component);
}