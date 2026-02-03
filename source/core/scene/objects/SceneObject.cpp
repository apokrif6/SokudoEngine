#include "SceneObject.h"
#include "yaml-cpp/node/node.h"
#include <algorithm>
#include "core/components/ComponentFactory.h"
#include "core/components/TransformComponent.h"

void Core::Scene::SceneObject::update(Renderer::VkRenderData& renderData)
{
    for (auto& component : mComponents)
    {
        component->update(renderData);
    }
}

void Core::Scene::SceneObject::draw(Renderer::VkRenderData& renderData)
{
    for (auto& component : mComponents)
    {
        component->draw(renderData);
    }
}

void Core::Scene::SceneObject::cleanup(Renderer::VkRenderData& renderData)
{
    for (auto& component : mComponents)
    {
        component->onRemoved();
        component->cleanup(renderData);
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

    return node;
}

void Core::Scene::SceneObject::deserialize(const YAML::Node& node)
{
    mName = node["name"].as<std::string>();

    const auto componentsNode = node["components"];
    if (!componentsNode)
    {
        return;
    }

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