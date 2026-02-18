#include "Scene.h"

#include "components/TransformComponent.h"

void Core::Scene::Scene::addObject(std::shared_ptr<SceneObject> object)
{
    mObjects.emplace_back(std::move(object));
    if (sceneObjectSelection.selectedObject.expired())
    {
        sceneObjectSelection.selectedObject = mObjects.back();
    }
}

std::shared_ptr<Core::Scene::SceneObject> Core::Scene::Scene::createObject(const std::string& name)
{
    auto newObject = std::make_shared<SceneObject>(name);
    newObject->addComponent<Component::TransformComponent>();
    addObject(newObject);

    return newObject;
}

void Core::Scene::Scene::update(Renderer::VkRenderData& renderData, float deltaTime)
{
    mUpdateSceneProfilingTimer.start();
    for (auto& object : mObjects)
    {
        object->update(renderData);
    }
    renderData.rdUpdateSceneProfilingTime = mUpdateSceneProfilingTimer.stop();
}

void Core::Scene::Scene::draw(Renderer::VkRenderData& renderData)
{
    for (auto& object : mObjects)
    {
        object->draw(renderData);
    }
}
void Core::Scene::Scene::cleanup(Renderer::VkRenderData& renderData)
{
    for (auto& object : mObjects)
    {
        object->cleanup(renderData);
    }
}
