#pragma once

#include "string"
#include "vk-renderer/buffers/VertexBuffer.h"
#include "vk-renderer/buffers/IndexBuffer.h"
#include "utils/ShapeUtils.h"

namespace Core::Renderer
{
class Primitive
{
public:
    Primitive(const std::vector<Vertex>& vertexBufferData, const std::vector<uint32_t>& indexBufferData,
              const std::unordered_map<aiTextureType, VkTextureData>& textures, const MaterialInfo& materialInfo,
              const Animations::BonesInfo& bonesInfo, VkRenderData& renderData, VkDescriptorSet materialDescriptorSet);

    void uploadVertexBuffer(VkRenderData& renderData);

    void uploadIndexBuffer(VkRenderData& renderData);

    void uploadUniformBuffer(VkRenderData& renderData, const glm::mat4& modelMatrix);

    void draw(const VkRenderData& renderData);

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

    const std::unordered_map<aiTextureType, VkTextureData> mTextures;
    VkTextureData mAlbedoTexture{};

    VkUniformBufferData mMaterialUBO{};
    MaterialInfo mMaterialInfo{};
    VkDescriptorSet mMaterialDescriptorSet{};

    Animations::BonesInfo mBonesInfo{};

    VkUniformBufferData mPrimitiveDataUBO{};

    PrimitiveFlagsPushConstants primitiveFlagsPushConstants{};
};
} // namespace Core::Renderer