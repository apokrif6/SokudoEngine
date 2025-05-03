#pragma once

#include <vector>
#include <glm/fwd.hpp>
#include "core/vk-renderer/Mesh.h"

// TODO
// this is circular dependency, bro fix it
// pull out animator from mesh and make it singleton manager
namespace Core::Renderer
{
class Mesh;
}

namespace Core::Animations
{
class Animator
{
  public:
    void update(Renderer::Mesh* mesh);

    void updateBonesTransform(Renderer::Mesh* mesh);
};
} // namespace Core::Animations