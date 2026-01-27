#pragma once

#include <memory>
#include "core/scene/objects/SceneObject.h"
#include "SceneEditor.h"
#include "core/tools/Timer.h"
#include "core/system/System.h"
#include "core/system/Updatable.h"
#include "core/system/Drawable.h"

namespace Core::Scene
{
class Scene : public System::ISystem, public System::IUpdatable, public System::IDrawable
{
public:
    void addObject(std::shared_ptr<SceneObject> object);

    std::shared_ptr<SceneObject> createObject(const std::string& name);

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