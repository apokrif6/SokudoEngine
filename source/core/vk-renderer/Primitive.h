#pragma once

#include "string"
#include "core/vk-renderer/buffers/VertexBuffer.h"
#include "core/vk-renderer/buffers/IndexBuffer.h"
#include "core/utils/ShapeUtils.h"

namespace Core::Renderer
{
class Primitive
{
public:
    Primitive(const std::vector<NewVertex>& vertexBufferData, const std::vector<uint32_t>& indexBufferData,
              const std::unordered_map<aiTextureType, VkTextureData>& textures,
              const MaterialInfo& materialInfo, const Animations::BonesInfo& bonesInfo,
              VkRenderData& renderData, VkDescriptorSet materialDescriptorSet);

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

    void createBonesTransformBuffer(VkRenderData& renderData);

    void createModelMatrixBuffer(VkRenderData& renderData);

    void createCameraBuffer(VkRenderData& renderData);

    void createLightsBuffer(VkRenderData& renderData);

    VkPrimitiveRenderData primitiveRenderData;

    std::vector<NewVertex> mVertexBufferData;
    std::vector<uint32_t> mIndexBufferData;

    const std::unordered_map<aiTextureType, VkTextureData> mTextures;
    VkTextureData mAlbedoTexture{};

    VkUniformBufferData mMaterialUBO{};
    MaterialInfo mMaterialInfo{};
    VkDescriptorSet mMaterialDescriptorSet{};

    VkUniformBufferData mBonesTransformUBO{};
    Animations::BonesInfo mBonesInfo{};

    VkUniformBufferData mModelUBO{};

    VkUniformBufferData mCameraUBO{};

    VkUniformBufferData mLightsUBO{};
    LightsInfo lightsData{};

    PrimitiveFlagsPushConstants primitiveFlagsPushConstants;
};
} // namespace Core::Renderer