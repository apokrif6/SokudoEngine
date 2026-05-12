#include "Scene.h"

#include "components/TransformComponent.h"

void Core::Scene::Scene::addObject(std::shared_ptr<SceneObject> object)
{
    registerObjectRecursive(object);

    mObjects.emplace_back(std::move(object));
    if (sceneObjectSelection.selectedObject.expired())
    {
        sceneObjectSelection.selectedObject = mObjects.back();
    }
}

std::shared_ptr<Core::Scene::SceneObject> Core::Scene::Scene::createObject(const std::string& name)
{
    auto newObject = std::make_shared<SceneObject>(name, this);
    newObject->addComponent<Component::TransformComponent>();
    addObject(newObject);

    return newObject;
}

void Core::Scene::Scene::removeObject(const std::shared_ptr<SceneObject>& object, Renderer::VkRenderData& renderData)
{
    if (sceneObjectSelection.selectedObject.lock() == object)
    {
        sceneObjectSelection.selectedObject.reset();
    }

    unregisterObjectRecursive(object.get());

    object->cleanup(renderData);

    if (auto* parent = object->getParent())
    {
        parent->removeChild(object.get());
    }
    else
    {
        mObjects.erase(std::remove(mObjects.begin(), mObjects.end(), object), mObjects.end());
    }
}

void Core::Scene::Scene::registerObjectRecursive(const std::shared_ptr<SceneObject>& object)
{
    if (!object)
    {
        return;
    }

    mUUIDToSceneObjects[object->getUUID()] = object.get();

    for (const auto& component : object->getComponents())
    {
        registerComponent(component.get());
    }

    for (const auto& child : object->getChildren())
    {
        registerObjectRecursive(child);
    }
}

void Core::Scene::Scene::unregisterObjectRecursive(const SceneObject* object)
{
    if (!object)
    {
        return;
    }

    mUUIDToSceneObjects.erase(object->getUUID());

    for (const auto& component : object->getComponents())
    {
        unregisterComponent(component.get());
    }

    for (const auto& child : object->getChildren())
    {
        unregisterObjectRecursive(child.get());
    }
}

void Core::Scene::Scene::registerComponent(Component::Component* component)
{
    mUUIDToComponents[component->getUUID()] = component;
}

void Core::Scene::Scene::unregisterComponent(Component::Component* component)
{
    mUUIDToComponents.erase(component->getUUID());
}

void Core::Scene::Scene::update(Renderer::VkRenderData& renderData, float deltaTime)
{
    // TODO
    // where to move this?
    renderData.rdGlobalSceneData.lightCount.x = 0;

    mUpdateSceneProfilingTimer.start();
    for (const auto& object : mObjects)
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
    mObjects.clear();
    mUUIDToSceneObjects.clear();
    mUUIDToComponents.clear();
}
