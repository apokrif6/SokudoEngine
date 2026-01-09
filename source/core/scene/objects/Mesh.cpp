#include "Mesh.h"
#include "core/vk-renderer/buffers/UniformBuffer.h"
#include "core/engine/Engine.h"
#include "core/animations/AnimationsUtils.h"
#include "core/components/TransformComponent.h"

void buildDebugSkeletonLines(const Core::Animations::Skeleton& skeleton, const Core::Animations::BonesInfo& bonesInfo,
                             std::vector<Core::Renderer::Debug::DebugBone>& debugBones,
                             const Core::Animations::BoneNode& node, const glm::mat4& parentTransform,
                             const glm::mat4& meshTransform = glm::mat4(1.f))
{
    glm::mat4 currentTransform;
    if (bonesInfo.boneNameToIndexMap.contains(node.name))
    {
        int boneIndex = bonesInfo.boneNameToIndexMap.at(node.name);
        currentTransform = meshTransform * bonesInfo.bones[boneIndex].animatedGlobalTransform;
    }
    else
    {
        currentTransform = parentTransform * node.localTransform;
    }

    glm::vec3 parentPos = glm::vec3(parentTransform * glm::vec4(0, 0, 0, 1));
    glm::vec3 currentPos = glm::vec3(currentTransform * glm::vec4(0, 0, 0, 1));

    if (parentTransform != glm::mat4(1.f))
    {
        debugBones.push_back({parentPos, currentPos});
    }

    for (const auto& child : node.children)
    {
        buildDebugSkeletonLines(skeleton, bonesInfo, debugBones, child, currentTransform, meshTransform);
    }
}

Core::Renderer::Mesh::Mesh(Animations::Skeleton skeleton)
    : mSkeleton(std::move(skeleton))
{
}

Core::Renderer::Mesh::~Mesh() { Logger::log(1, "%s: Destroyed mesh for owner %s", __FUNCTION__, getOwner()->getName().c_str()); }

void Core::Renderer::Mesh::onAddedToScene()
{
    Logger::log(1, "%s: Mesh %s added to scene for owner", __FUNCTION__, getOwner()->getName().c_str());

    Engine::getInstance().getSystem<Animations::Animator>()->addMesh(this);
}

void Core::Renderer::Mesh::onRemovedFromScene()
{
    Logger::log(1, "%s: Mesh %s removed from scene for owner", __FUNCTION__, getOwner()->getName().c_str());

    Engine::getInstance().getSystem<Animations::Animator>()->removeMesh(this);
}

void Core::Renderer::Mesh::addPrimitive(const std::vector<NewVertex>& vertexBufferData,
                                        const std::vector<uint32_t>& indexBufferData,
                                        const std::unordered_map<aiTextureType, VkTextureData>& textures,
                                        VkRenderData& renderData, const MaterialInfo& materialInfo,
                                        const Animations::BonesInfo& bonesInfo, VkDescriptorSet materialDescriptorSet)
{
    mPrimitives.emplace_back(vertexBufferData, indexBufferData, textures, materialInfo, bonesInfo, renderData,
                             materialDescriptorSet);
}

void Core::Renderer::Mesh::update(VkRenderData& renderData)
{
    // TODO
    // should be replaced with local transform, like SceneComponent
    const auto* transformComponent =
        getOwner()->getComponent<Core::Component::TransformComponent>();

    if (!transformComponent)
    {
        Logger::log(3, "%s: Warning! Mesh %s has no TransformComponent!", __FUNCTION__, getOwner()->getName().c_str());
        return;

    }

    for (auto& primitive : mPrimitives)
    {
        primitive.uploadVertexBuffer(renderData);
        primitive.uploadIndexBuffer(renderData);
        primitive.uploadUniformBuffer(renderData, transformComponent->transform.getMatrix());

        if (shouldDrawDebugSkeleton())
        {
            std::vector<Debug::DebugBone> debugBones;
            buildDebugSkeletonLines(mSkeleton, primitive.getBonesInfo(), debugBones, mSkeleton.getRootNode(),
                                    transformComponent->transform.getMatrix(), transformComponent->transform.getMatrix());
            mSkeleton.updateDebug(renderData, debugBones);
        }
    }
}

void Core::Renderer::Mesh::draw(VkRenderData& renderData)
{
    for (auto& primitive : mPrimitives)
    {
        primitive.draw(renderData);

        if (shouldDrawDebugSkeleton())
        {
            mSkeleton.drawDebug(renderData);
        }
    }
}

void Core::Renderer::Mesh::cleanup(VkRenderData& renderData)
{
    for (auto& primitive : mPrimitives)
    {
        primitive.cleanup(renderData);
    }

    mSkeleton.cleanup(renderData);
}

YAML::Node Core::Renderer::Mesh::serialize() const
{
    YAML::Node node = getOwner()->serialize();
    node["meshFile"] = mMeshFilePath;
    for (auto& animPath : mAnimationFiles)
    {
        node["animations"].push_back(animPath);
    }
    node["shouldPlayAnimation"] = mShouldPlayAnimation;
    node["currentAnimationIndex"] = mCurrentAnimationIndex;
    return node;
}

void Core::Renderer::Mesh::deserialize(const YAML::Node& node)
{
    getOwner()->deserialize(node);
    mMeshFilePath = node["meshFile"].as<std::string>();
    mAnimationFiles.clear();
    if (node["animations"])
    {
        for (auto& animNode : node["animations"])
        {
            mAnimationFiles.push_back(animNode.as<std::string>());
        }
    }
    mShouldPlayAnimation = node["shouldPlayAnimation"].as<bool>();
    mCurrentAnimationIndex = node["currentAnimationIndex"].as<uint16_t>();

    // I don't like this here, it should be done somewhere else (maybe time for a resource manager hehe?)
    auto& renderData = Engine::getInstance().getRenderData();
    Utils::MeshData meshData = Utils::loadMeshFromFile(mMeshFilePath, renderData);

    std::vector<Animations::AnimationClip> animations;
    for (auto& animPath : mAnimationFiles)
    {
        animations.push_back(Animations::AnimationsUtils::loadAnimationFromFile(animPath));
    }
    meshData.animations = animations;
    mSkeleton = meshData.skeleton;
    setupAnimations(meshData.animations);
    initDebugSkeleton(renderData);

    for (auto& primitive : meshData.primitives)
    {
        addPrimitive(primitive.vertices, primitive.indices, primitive.textures, renderData, primitive.material,
                     primitive.bones, primitive.materialDescriptorSet);
    }
}
