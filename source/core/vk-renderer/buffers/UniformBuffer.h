#pragma once

#include <vulkan/vulkan.h>

#include "core/vk-renderer/VkRenderData.h"

namespace Core::Renderer
{
class UniformBuffer
{
public:
    static bool init(VkRenderData& renderData, VkUniformBufferData& UBOData, size_t bufferSize,
                     const std::string& name);

    static void uploadData(VkRenderData& renderData, VkUniformBufferData& UBOData,
                           const glm::vec3& vector);

    static void uploadData(VkRenderData& renderData, VkUniformBufferData& UBOData,
                           const glm::mat4& matrix);

    static void uploadData(VkRenderData& renderData, VkUniformBufferData& UBOData,
                           std::vector<glm::mat4> matrices);

    static void uploadData(VkRenderData& renderData, VkUniformBufferData& UBOData,
                           const MaterialInfo& materialInfo);

    static void uploadData(VkRenderData& renderData, VkUniformBufferData& UBOData,
                           const CameraInfo& cameraInfo);

    static void uploadData(VkRenderData& renderData, VkUniformBufferData& UBOData,
                           const LightsInfo& lightsInfo);

    static void uploadData(VkRenderData& renderData, VkUniformBufferData& UBOData,
                           const CaptureInfo& captureInfo);

    static void cleanup(VkRenderData& renderData, VkUniformBufferData& UBOData);
};
} // namespace Core::Renderer