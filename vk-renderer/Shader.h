#pragma once

#include <vulkan/vulkan.h>
#include <string>

class Shader
{
  public:
    static VkShaderModule loadShader(VkDevice device, std::string shaderFileName);

  private:
    static std::string loadFileToString(std::string fileName);
};
