#pragma once

#include <string>
#include <glm/fwd.hpp>
#include "vk-renderer/VkRenderData.h"
#include "unordered_map"
#include "animations/AnimationsData.h"
#include "assimp/material.h"

namespace Core::Assets
{
class TextureAsset;
}

namespace Core::Utils
{
// TODO
// move this struct to a more general place, since it can be used by other systems as well
struct PrimitiveData
{
    std::vector<Renderer::Vertex> vertices;
    std::vector<uint32_t> indices;
    std::unordered_map<aiTextureType, std::shared_ptr<Assets::TextureAsset>> textures;
    Renderer::MaterialInfo material;
    // TODO
    // should it be part of MaterialInfo?
    VkDescriptorSet materialDescriptorSet{};
    Animations::BonesInfo bones;
};

struct MeshNode
{
    std::string name;
    glm::mat4 localTransform;
    std::vector<PrimitiveData> primitives;
    std::vector<MeshNode> children;
};

struct SkeletonData
{
    Animations::BoneNode rootNode;
};

struct MeshData
{
    MeshNode rootNode;
    SkeletonData skeletonData;
    std::vector<Animations::AnimationClip> animations;
};

MeshData loadMeshFromFile(const std::string& fileName, Renderer::VkRenderData& renderData);

void collectPrimitivesRecursive(const MeshNode& node, const glm::mat4& parentTransform,
                                std::vector<PrimitiveData>& outAllPrimitives);

void createSpritePrimitiveData(const std::string& spritePath, Renderer::VkRenderData& renderData,
                               PrimitiveData& outPrimitiveData);
} // namespace Core::Utils
