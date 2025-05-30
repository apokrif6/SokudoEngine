#pragma once

#include <vulkan/vulkan.h>
#include <string>
#include <vector>

namespace Core::Renderer
{
class Shader
{
  public:
    static VkShaderModule loadShader(VkDevice device, const std::string& shaderFileName);

    static std::vector<VkVertexInputAttributeDescription> getAttributeDescriptionsFromSPV(const std::string& shaderFileName);

  private:
    static std::string loadFileToString(const std::string& fileName);

    static std::vector<char> readBinaryFile(const std::string& fileName);
};
}