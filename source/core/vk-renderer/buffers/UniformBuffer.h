#pragma once

#include "core/engine/Engine.h"
#include "core/vk-renderer/VkRenderData.h"

namespace Core::Renderer
{
class UniformBuffer
{
public:
    static bool init(VkRenderData& renderData, VkUniformBufferData& UBOData, size_t bufferSize,
                     const std::string& name, VkDescriptorSetLayout customLayout = VK_NULL_HANDLE,
                     std::vector<VkDescriptorPoolSize> extraPoolSizes = {});

    static void uploadData(VkRenderData& renderData, VkUniformBufferData& UBOData, const glm::vec3& vector);

    static void uploadData(VkRenderData& renderData, VkUniformBufferData& UBOData, const glm::mat4& matrix);

    static void uploadData(VkRenderData& renderData, VkUniformBufferData& UBOData, std::vector<glm::mat4> matrices);

    // TODO
    // should be templated
    static void uploadData(VkRenderData& renderData, VkUniformBufferData& UBOData, const GlobalSceneData& globalSceneData);

    static void uploadData(VkRenderData& renderData, VkUniformBufferData& UBOData, const MaterialInfo& materialInfo);

    static void uploadData(VkRenderData& renderData, VkUniformBufferData& UBOData, const PrimitiveData& primitiveData);

    static void uploadData(VkRenderData& renderData, VkUniformBufferData& UBOData, const CaptureInfo& captureInfo);

    static void cleanup(VkRenderData& renderData, VkUniformBufferData& UBOData);
};
} // namespace Core::Renderer