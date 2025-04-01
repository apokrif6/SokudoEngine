#pragma once

#include <future>
#include <string>
#include <vulkan/vulkan.h>

#include "VkRenderData.h"

namespace Core::Renderer
{
class Texture
{
  public:
    static std::future<bool> loadTexture(Core::Renderer::VkRenderData& renderData, VkTextureData& textureData,
                                         const std::string& textureFilename);

    static std::future<bool> loadTextures(Core::Renderer::VkRenderData& renderData,
                                          VkTextureArrayData& textureArrayData,
                                          const std::vector<std::string>& textureFilenames);

    static void cleanup(Core::Renderer::VkRenderData& renderData, VkTextureData& textureData);
};
} // namespace Core::Renderer