#include "Scene.h"

void Core::Scene::Scene::addObject(std::shared_ptr<SceneObject> object)
{
    mObjects.emplace_back(std::move(object));
}

void Core::Scene::Scene::update(Core::Renderer::VkRenderData& renderData, float deltaTime)
{
    for (auto& object : mObjects)
    {
        object->update(renderData);
    }
}

void Core::Scene::Scene::draw(Core::Renderer::VkRenderData& renderData)
{
    for (auto& object : mObjects)
    {
        object->draw(renderData);
    }
}
void Core::Scene::Scene::cleanup(Core::Renderer::VkRenderData& renderData)
{
    for (auto& object : mObjects)
    {
        object->cleanup(renderData);
    }
}
