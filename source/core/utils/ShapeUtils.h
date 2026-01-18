#pragma once

#include <string>
#include <glm/fwd.hpp>
#include <memory>
#include "core/vk-renderer/VkRenderData.h"
#include "unordered_map"
#include "core/animations/AnimationsData.h"
#include "assimp/Importer.hpp"
#include "assimp/material.h"
#include "core/animations/Skeleton.h"

namespace Core::Utils
{
struct PrimitiveData
{
    std::vector<Renderer::Vertex> vertices;
    std::vector<uint32_t> indices;
    std::unordered_map<aiTextureType, Renderer::VkTextureData> textures;
    Renderer::MaterialInfo material;
    // TODO
    // should it be part of MaterialInfo?
    VkDescriptorSet materialDescriptorSet{};
    Animations::BonesInfo bones;
};

struct MeshData
{
    std::vector<PrimitiveData> primitives;
    Animations::Skeleton skeleton;
    std::vector<Animations::AnimationClip> animations;
};

MeshData loadMeshFromFile(const std::string& fileName, Renderer::VkRenderData& renderData);
} // namespace Core::Utils
