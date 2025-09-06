#pragma once

#include <memory>
#include "SceneObject.h"
#include "SceneEditor.h"
#include "core/tools/Timer.h"

namespace Core::Scene
{
class Scene
{
  public:
    void addObject(std::shared_ptr<SceneObject> object);

    void update(Core::Renderer::VkRenderData& renderData, float deltaTime);

    void draw(Core::Renderer::VkRenderData& renderData);

    void cleanup(Core::Renderer::VkRenderData& renderData);

    [[nodiscard]] std::vector<std::shared_ptr<SceneObject>> getObjects() const { return mObjects; }

    [[nodiscard]] SceneObjectSelection& getSceneObjectSelection() { return sceneObjectSelection; }

  private:
    std::vector<std::shared_ptr<SceneObject>> mObjects;

    SceneObjectSelection sceneObjectSelection;

    Timer mUpdateSceneProfilingTimer;
};
} // namespace Core::Scene