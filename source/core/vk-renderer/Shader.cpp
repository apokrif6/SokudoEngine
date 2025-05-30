#include <fstream>
#include <cstring>
#include <algorithm>
#include "Shader.h"
#include "core/tools/Logger.h"
#include "spirv-reflect/spirv_reflect.h"

#define SPV_REFLECTINTERFACEVARIABLE_LOCATION_NOTFOUND 0xFFFFFFFF

VkShaderModule Core::Renderer::Shader::loadShader(VkDevice device, const std::string& shaderFileName)
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

std::vector<VkVertexInputAttributeDescription>
Core::Renderer::Shader::getAttributeDescriptionsBySpvReflect(const std::string& shaderFileName)
{
    std::vector<char> shaderCode = readBinaryFile(shaderFileName);

    SpvReflectShaderModule module;
    SpvReflectResult result = spvReflectCreateShaderModule(shaderCode.size(), shaderCode.data(), &module);
    if (result != SPV_REFLECT_RESULT_SUCCESS)
        throw std::runtime_error("SPIR-V reflection failed to create shader module");

    uint32_t inputVarCount = 0;
    result = spvReflectEnumerateInputVariables(&module, &inputVarCount, nullptr);
    if (result != SPV_REFLECT_RESULT_SUCCESS)
        throw std::runtime_error("Failed to enumerate input variables with SPIR-V reflection");

    std::vector<SpvReflectInterfaceVariable*> inputVariables(inputVarCount);
    spvReflectEnumerateInputVariables(&module, &inputVarCount, inputVariables.data());

    std::sort(inputVariables.begin(), inputVariables.end(),
              [](const SpvReflectInterfaceVariable* a, const SpvReflectInterfaceVariable* b)
              { return a->location < b->location; });

    std::vector<VkVertexInputAttributeDescription> attributeDescriptions;
    uint32_t currentOffset = 0;

    for (const SpvReflectInterfaceVariable* inputVariable : inputVariables)
    {
        if (inputVariable->location == SPV_REFLECTINTERFACEVARIABLE_LOCATION_NOTFOUND)
        {
            Logger::log(1, "%s: input variable '%s' has no location assigned, skipping\n",
                        __FUNCTION__, inputVariable->name);
            continue;
        }

        VkVertexInputAttributeDescription inputAttributeDescription{};
        inputAttributeDescription.binding = 0;
        inputAttributeDescription.location = inputVariable->location;
        inputAttributeDescription.format = static_cast<VkFormat>(inputVariable->format);
        inputAttributeDescription.offset = currentOffset;

        uint32_t formatSize;
        switch (inputAttributeDescription.format)
        {
        case VK_FORMAT_R32_SFLOAT:
            formatSize = 4;
            break;
        case VK_FORMAT_R32G32_SFLOAT:
            formatSize = 8;
            break;
        case VK_FORMAT_R32G32B32_SFLOAT:
            formatSize = 12;
            break;
        case VK_FORMAT_R32G32B32A32_SFLOAT:
            formatSize = 16;
            break;
        case VK_FORMAT_R32G32B32A32_SINT:
            formatSize = 16;
            break;
        default:
            throw std::runtime_error("Unsupported or unknown inputAttributeDescription format");
        }

        attributeDescriptions.push_back(inputAttributeDescription);
        currentOffset += formatSize;

        attributeDescriptions.push_back(inputAttributeDescription);
    }

    spvReflectDestroyShaderModule(&module);

    return attributeDescriptions;
}

std::string Core::Renderer::Shader::loadFileToString(const std::string& fileName)
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

std::vector<char> Core::Renderer::Shader::readBinaryFile(const std::string& fileName)
{
    std::ifstream file(fileName, std::ios::ate | std::ios::binary);
    if (!file.is_open())
        throw std::runtime_error("failed to open file: " + fileName);

    size_t fileSize = static_cast<size_t>(file.tellg());
    std::vector<char> buffer(fileSize);

    file.seekg(0);
    file.read(buffer.data(), fileSize);
    file.close();

    return buffer;
}
