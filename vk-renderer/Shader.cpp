#include <fstream>
#include "Shader.h"
#include "Logger.h"

VkShaderModule Shader::loadShader(VkDevice device, const std::string& shaderFileName)
{
    std::string shaderAsText;
    shaderAsText = loadFileToString(shaderFileName);

    VkShaderModuleCreateInfo shaderCreateInfo{};
    shaderCreateInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    shaderCreateInfo.codeSize = shaderAsText.size();
    shaderCreateInfo.pCode = reinterpret_cast<const uint32_t*>(shaderAsText.c_str());

    VkShaderModule shaderModule;
    if (vkCreateShaderModule(device, &shaderCreateInfo, nullptr, &shaderModule) != VK_SUCCESS)
    {
        Logger::log(1, "%s: could not load shader '%s'\n", __FUNCTION__, shaderFileName.c_str());
        return VK_NULL_HANDLE;
    }

    return shaderModule;
}

std::string Shader::loadFileToString(const std::string& fileName)
{
    std::ifstream inFile(fileName, std::ios::binary);
    std::string str;

    if (inFile.is_open())
    {
        str.clear();
        inFile.seekg(0, std::ios::end);
        str.reserve(inFile.tellg());
        inFile.seekg(0, std::ios::beg);

        str.assign((std::istreambuf_iterator<char>(inFile)), std::istreambuf_iterator<char>());
        inFile.close();
    }
    else
    {
        Logger::log(1, "%s error: could not open file %s\n", __FUNCTION__, fileName.c_str());
        Logger::log(1, "%s error: system says '%s'\n", __FUNCTION__, strerror(errno));
        return {};
    }

    if (inFile.bad() || inFile.fail())
    {
        Logger::log(1, "%s error: error while reading file %s\n", __FUNCTION__, fileName.c_str());
        inFile.close();
        return {};
    }

    inFile.close();
    Logger::log(1, "%s: file %s successfully read to string\n", __FUNCTION__, fileName.c_str());
    return str;
}
