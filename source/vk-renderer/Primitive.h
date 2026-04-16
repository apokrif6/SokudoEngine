#pragma once

#include "VkRenderData.h"
#include "animations/AnimationsData.h"
#include <memory>
#include <unordered_map>
#include <vector>
#include <assimp/material.h>
#include <glm/fwd.hpp>

namespace Core::Assets
{
class TextureAsset;
}
namespace Core::Renderer
{
struct MaterialInfo;
struct Vertex;
enum PrimitiveRenderType
{
    PBR,
    Sprite,
    Unlit
};

class Primitive
{
public:
    Primitive(const std::vector<Vertex>& vertexBufferData, const std::vector<uint32_t>& indexBufferData,
              const std::unordered_map<aiTextureType, std::shared_ptr<Assets::TextureAsset>>& textures,
              const MaterialInfo& materialInfo, VkDescriptorSet materialDescriptorSet,
              const Animations::BonesInfo& bonesInfo, VkRenderData& renderData);

    void uploadVertexBuffer(VkRenderData& renderData);

    void uploadIndexBuffer(VkRenderData& renderData);

    void uploadUniformBuffer(VkRenderData& renderData, const glm::mat4& modelMatrix);

    void draw(const VkRenderData& renderData, PrimitiveRenderType renderType = PBR);

    void cleanup(VkRenderData& renderData);

    Animations::BonesInfo& getBonesInfo() { return mBonesInfo; }

private:
    void createVertexBuffer(VkRenderData& renderData);

    void createIndexBuffer(VkRenderData& renderData);

    void createMaterialBuffer(VkRenderData& renderData);

    void createPrimitiveDataBuffer(VkRenderData& renderData);

    VkPrimitiveRenderData primitiveRenderData;

    std::vector<Vertex> mVertexBufferData;
    std::vector<uint32_t> mIndexBufferData;

    const std::unordered_map<aiTextureType, std::shared_ptr<Assets::TextureAsset>> mTextures;
    VkTextureData mAlbedoTexture{};

    VkUniformBufferData mMaterialUBO{};
    MaterialInfo mMaterialInfo{};
    VkDescriptorSet mMaterialDescriptorSet{};

    Animations::BonesInfo mBonesInfo{};

    VkUniformBufferData mPrimitiveDataUBO{};

    PrimitiveFlagsPushConstants primitiveFlagsPushConstants{};
};
} // namespace Core::Renderer