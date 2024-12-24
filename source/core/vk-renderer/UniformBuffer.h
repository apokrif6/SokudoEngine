#pragma once

#include <vulkan/vulkan.h>

#include "VkRenderData.h"

namespace Core::Renderer
{
class UniformBuffer
{
  public:
    static bool init(Core::Renderer::VkRenderData& renderData);

    static void uploadData(Core::Renderer::VkRenderData& renderData, VkUploadMatrices matrices);

    static void cleanup(Core::Renderer::VkRenderData& renderData);
};
}