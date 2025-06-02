#pragma once

#include "assimp/scene.h"
#include "assimp/Importer.hpp"
#include "core/vk-renderer/VkRenderData.h"
#include "core/vk-renderer/Mesh.h"
#include <map>
#include <memory>

// TODO
// create templated class for all singletons
namespace Core::Animations
{
class AnimatorSingleton
{
  public:
    static AnimatorSingleton& getInstance()
    {
        static AnimatorSingleton instance;
        return instance;
    }

    AnimatorSingleton(const AnimatorSingleton&) = delete;
    AnimatorSingleton& operator=(const AnimatorSingleton&) = delete;

    void update(const Renderer::VkRenderData& renderData, Renderer::Mesh* mesh);

    void updateBonesTransform(Renderer::Mesh* mesh);

    void readNodeHierarchy(Renderer::Mesh* mesh, const aiNode* node, const glm::mat4& parentTransform,
                           const aiAnimation* animation, float animationTime);

    // TODO
    // dude refactor this shit ( ｡ •̀ ᴖ •́ ｡)
    std::map<std::string, std::shared_ptr<Assimp::Importer>> importers;
  private:
    AnimatorSingleton() = default;
    ~AnimatorSingleton() = default;

    const aiNodeAnim* findAnimationNode(const aiAnimation* animation, const std::string& nodeName);

    glm::vec3 interpolatePosition(const aiNodeAnim* nodeAnim, float animationTime);

    glm::quat interpolateRotation(const aiNodeAnim* nodeAnim, float animationTime);

    glm::vec3 interpolateScaling(const aiNodeAnim* nodeAnim, float animationTime);

    glm::mat4 mGlobalInverseTransform;

    float mAnimationTime = 0.f;
};
} // namespace Core::Animations
