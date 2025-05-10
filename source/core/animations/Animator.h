#pragma once

#include <vector>
#include <glm/fwd.hpp>
#include "core/vk-renderer/Mesh.h"
#include "assimp/scene.h"

// TODO
// this is circular dependency, bro fix it
// pull out animator from mesh and make it singleton manager with list of assimp scenes to animate
namespace Core::Renderer
{
class Mesh;
}

namespace Core::Animations
{
class Animator
{
  public:
    void update(const Renderer::VkRenderData& renderData, Renderer::Mesh* mesh);

    void updateBonesTransform(Renderer::Mesh* mesh);

    void readNodeHierarchy(Renderer::Mesh* mesh, const aiNode* node, const glm::mat4& parentTransform,
                           const aiAnimation* animation, float animationTime);

  private:
    const aiNodeAnim* findAnimationNode(const aiAnimation* animation, const std::string& nodeName);

    glm::vec3 interpolatePosition(const aiNodeAnim* nodeAnim, float animationTime);

    glm::quat interpolateRotation(const aiNodeAnim* nodeAnim, float animationTime);

    glm::vec3 interpolateScaling(const aiNodeAnim* nodeAnim, float animationTime);

    glm::mat4 mGlobalInverseTransform;

    float mAnimationTime = 0.f;
};
} // namespace Core::Animations