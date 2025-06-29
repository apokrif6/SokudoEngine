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
    std::vector<Core::Renderer::NewVertex> vertices;
    std::vector<uint32_t> indices;
    std::unordered_map<aiTextureType, Core::Renderer::VkTextureData> textures;
    Core::Renderer::MaterialInfo material;
    Core::Animations::BonesInfo bones;
};

struct MeshData
{
    std::vector<PrimitiveData> primitives;
    Core::Animations::Skeleton skeleton;
    std::vector<Core::Animations::AnimationClip> animations;
};

MeshData loadMeshFromFile(const std::string& fileName, Core::Renderer::VkRenderData& renderData);
} // namespace Core::Utils
