#pragma once

#include <vulkan/vulkan.h>
#include <string>

class Shader
{
  public:
    static VkShaderModule loadShader(VkDevice device, const std::string& shaderFileName);

  private:
    static std::string loadFileToString(const std::string& fileName);
};
