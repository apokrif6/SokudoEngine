#pragma once

#include <memory>
#include "scene/objects/SceneObject.h"
#include "SceneEditor.h"
#include "tools/Timer.h"
#include "system/System.h"
#include "system/Updatable.h"
#include "system/Drawable.h"

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

    virtual void update(Renderer::VkRenderData& renderData, float deltaTime) override;

    virtual System::DrawLayer getDrawLayer() const override { return System::DrawLayer::World; }

    virtual void draw(Renderer::VkRenderData& renderData) override;

    void cleanup(Renderer::VkRenderData& renderData);

    [[nodiscard]] std::vector<std::shared_ptr<SceneObject>> getObjects() const { return mObjects; }

    [[nodiscard]] SceneObjectSelection& getSceneObjectSelection() { return sceneObjectSelection; }

private:
    std::vector<std::shared_ptr<SceneObject>> mObjects;

    SceneObjectSelection sceneObjectSelection;

    Timer mUpdateSceneProfilingTimer;
};
} // namespace Core::Scene