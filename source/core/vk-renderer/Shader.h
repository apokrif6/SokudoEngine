#pragma once

#include <vulkan/vulkan.h>
#include <string>

namespace Core::Renderer
{
class Shader
{
  public:
    static VkShaderModule loadShader(VkDevice device, const std::string& shaderFileName);

  private:
    static std::string loadFileToString(const std::string& fileName);
};
}