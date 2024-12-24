#pragma once

#include <string>
#include <vulkan/vulkan.h>

#include "VkRenderData.h"

namespace Core::Renderer
{
class Texture
{
  public:
    static bool loadTexture(Core::Renderer::VkRenderData renderData, VkTextureData& textureData, std::string textureFilename);

    static void cleanup(Core::Renderer::VkRenderData renderData, VkTextureData& textureData);
};
}