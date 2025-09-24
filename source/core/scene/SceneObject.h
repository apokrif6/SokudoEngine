#pragma once

#include <string>
#include "Transform.h"
#include "core/vk-renderer/VkRenderData.h"

namespace Core::Scene
{
class SceneObject
{
  public:
    explicit SceneObject(std::string name) : mName(std::move(name)) {}
    virtual ~SceneObject() = default;

    virtual void update(Core::Renderer::VkRenderData& renderData) {};
    virtual void draw(Core::Renderer::VkRenderData& renderData) {};
    virtual void cleanup(Core::Renderer::VkRenderData& renderData) {};

    Transform& getTransform() { return mTransform; }
    [[nodiscard]] const Transform& getTransform() const { return mTransform; }

    [[nodiscard]] const std::string& getName() const { return mName; }

  protected:
    std::string mName;
    Transform mTransform;
};
} // namespace Core::Scene