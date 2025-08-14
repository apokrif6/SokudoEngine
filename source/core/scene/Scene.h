#pragma once

#include <memory>
#include "SceneObject.h"

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

  private:
    std::vector<std::shared_ptr<SceneObject>> mObjects;
};
}