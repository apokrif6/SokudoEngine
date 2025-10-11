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
    Primitive(const std::vector<Renderer::NewVertex>& vertexBufferData, const std::vector<uint32_t>& indexBufferData,
              const std::unordered_map<aiTextureType, Renderer::VkTextureData>& textures,
              const Renderer::MaterialInfo& materialInfo, const Animations::BonesInfo& bonesInfo,
              Core::Renderer::VkRenderData& renderData, VkDescriptorSet materialDescriptorSet);

    void uploadVertexBuffer(Renderer::VkRenderData& renderData);

    void uploadIndexBuffer(Renderer::VkRenderData& renderData);

    void uploadUniformBuffer(Renderer::VkRenderData& renderData, const glm::mat4& modelMatrix);

    void draw(const Renderer::VkRenderData& renderData);

    void cleanup(Renderer::VkRenderData& renderData);

    Animations::BonesInfo& getBonesInfo() { return mBonesInfo; }

  private:
    void createVertexBuffer(Renderer::VkRenderData& renderData);

    void createIndexBuffer(Renderer::VkRenderData& renderData);

    void createMaterialBuffer(Renderer::VkRenderData& renderData);

    void createBonesTransformBuffer(Renderer::VkRenderData& renderData);

    void createModelMatrixBuffer(Renderer::VkRenderData& renderData);

    void createCameraBuffer(Renderer::VkRenderData& renderData);

    void createLightsBuffer(Renderer::VkRenderData& renderData);

    Renderer::VkPrimitiveRenderData primitiveRenderData;

    std::vector<Renderer::NewVertex> mVertexBufferData;
    std::vector<uint32_t> mIndexBufferData;

    const std::unordered_map<aiTextureType, Renderer::VkTextureData> mTextures;
    Renderer::VkTextureData mAlbedoTexture{};

    Renderer::VkUniformBufferData mMaterialUBO{};
    Renderer::MaterialInfo mMaterialInfo{};
    VkDescriptorSet mMaterialDescriptorSet{};

    Renderer::VkUniformBufferData mBonesTransformUBO{};
    Animations::BonesInfo mBonesInfo{};

    Renderer::VkUniformBufferData mModelUBO{};

    Renderer::VkUniformBufferData mCameraUBO{};

    Renderer::VkUniformBufferData mLightsUBO{};
    Renderer::LightsInfo lightsData{};

    Renderer::PrimitiveFlagsPushConstants primitiveFlagsPushConstants;
};
} // namespace Core::Renderer