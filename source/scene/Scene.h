#pragma once

#include <memory>
#include "scene/objects/SceneObject.h"
#include "SceneEditor.h"
#include "tools/Timer.h"
#include "system/System.h"
#include "system/Updatable.h"
#include "system/Drawable.h"

#include <ranges>

namespace Core::Scene
{
class Scene : public System::ISystem, public System::IUpdatable, public System::IDrawable
{
public:
    void addObject(std::shared_ptr<SceneObject> object);

    std::shared_ptr<SceneObject> createObject(const std::string& name);

    // how can I cleanup data without referencing renderData? maybe I can just mark object as deleted and then cleanup
    // in update?
    void removeObject(const std::shared_ptr<SceneObject>& object, Renderer::VkRenderData& renderData);

    void registerObjectRecursive(const std::shared_ptr<SceneObject>& object);

    void unregisterObjectRecursive(const SceneObject* object);

    void registerComponent(Component::Component* component);

    void unregisterComponent(Component::Component* component);

    void update(Renderer::VkRenderData& renderData, float deltaTime) override;

    [[nodiscard]] System::DrawLayer getDrawLayer() const override { return System::DrawLayer::World; }

    void draw(Renderer::VkRenderData& renderData) override;

    void cleanup(Renderer::VkRenderData& renderData);

    [[nodiscard]] std::vector<std::shared_ptr<SceneObject>> getObjects() const { return mObjects; }

    [[nodiscard]] SceneObjectSelection& getSceneObjectSelection() { return sceneObjectSelection; }

    template <typename T> T* findComponentInScene()
    {
        for (auto& object : mObjects)
        {
            if (T* component = findComponentRecursive<T>(object.get()))
            {
                return component;
            }
        }
        return nullptr;
    }

    template <typename T = SceneObject> T* findSceneObjectByUUID(const uuids::uuid& uuid)
    {
        const auto it = mUUIDToSceneObjects.find(uuid);
        if (it == mUUIDToSceneObjects.end())
        {
            return nullptr;
        }

        return dynamic_cast<T*>(it->second);
    }

    template <typename T = Component::Component> T* findComponentByUUID(const uuids::uuid& uuid)
    {
        const auto it = mUUIDToComponents.find(uuid);
        if (it == mUUIDToComponents.end())
        {
            return nullptr;
        }

        return dynamic_cast<T*>(it->second);
    }

    template <typename T> std::vector<T*> getAllComponentsOfType()
    {
        std::vector<T*> result;

        for (auto& component : mUUIDToComponents | std::views::values)
        {
            if (auto castedComponent = dynamic_cast<T*>(component))
            {
                result.push_back(castedComponent);
            }
        }

        return result;
    }

private:
    template <typename T> T* findComponentRecursive(SceneObject* object)
    {
        if (T* component = object->getComponent<T>())
        {
            return component;
        }

        for (auto& child : object->getChildren())
        {
            if (T* foundComponent = findComponentRecursive<T>(child.get()))
            {
                return foundComponent;
            }
        }

        return nullptr;
    }

    std::vector<std::shared_ptr<SceneObject>> mObjects;

    std::unordered_map<uuids::uuid, Component::Component*> mUUIDToComponents;
    std::unordered_map<uuids::uuid, SceneObject*> mUUIDToSceneObjects;

    SceneObjectSelection sceneObjectSelection;

    Timer mUpdateSceneProfilingTimer;
};
} // namespace Core::Scene