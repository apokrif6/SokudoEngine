#include "Scene.h"

void Core::Scene::Scene::addObject(std::shared_ptr<SceneObject> object)
{
    mObjects.emplace_back(std::move(object));
    if (sceneObjectSelection.selectedObject == nullptr)
    {
        sceneObjectSelection.selectedObject = mObjects[0];
    }
}

void Core::Scene::Scene::update(Core::Renderer::VkRenderData& renderData, float deltaTime)
{
    mUpdateSceneProfilingTimer.start();
    for (auto& object : mObjects)
    {
        object->update(renderData);
    }
    renderData.rdUpdateSceneProfilingTime = mUpdateSceneProfilingTimer.stop();
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
