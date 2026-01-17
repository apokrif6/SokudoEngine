#pragma once

#include <string>
#include "vulkan/vulkan_core.h"

namespace Core::Renderer::Debug
{
inline void setObjectName(VkDevice device, uint64_t handle, VkObjectType type, const std::string& name)
{
    VkDebugUtilsObjectNameInfoEXT nameInfo{};
    nameInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_OBJECT_NAME_INFO_EXT;
    nameInfo.objectType = type;
    nameInfo.objectHandle = handle;
    nameInfo.pObjectName = name.c_str();

    if (auto function = reinterpret_cast<PFN_vkSetDebugUtilsObjectNameEXT>(vkGetDeviceProcAddr(device, "vkSetDebugUtilsObjectNameEXT")))
    {
        function(device, &nameInfo);
    }
}

class Marker
{
public:
    Marker(VkDevice device, VkCommandBuffer cmd, const std::string& name, const float color[4])
        : mDevice(device), mCmd(cmd)
    {
        begin(mDevice, mCmd, name, color);
    }

    ~Marker()
    {
        end(mDevice, mCmd);
    }

    static void begin(VkDevice device, VkCommandBuffer cmd, const std::string& name, const float color[4])
    {
        VkDebugUtilsLabelEXT labelInfo{};
        labelInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_LABEL_EXT;
        labelInfo.pLabelName = name.c_str();
        memcpy(labelInfo.color, color, sizeof(float) * 4);

        if (auto function = reinterpret_cast<PFN_vkCmdBeginDebugUtilsLabelEXT>(vkGetDeviceProcAddr(device, "vkCmdBeginDebugUtilsLabelEXT")))
        {
            function(cmd, &labelInfo);
        }
    }

    static void end(VkDevice device, VkCommandBuffer cmd)
    {
        if (auto function = reinterpret_cast<PFN_vkCmdEndDebugUtilsLabelEXT>(vkGetDeviceProcAddr(device, "vkCmdEndDebugUtilsLabelEXT")))
        {
            function(cmd);
        }
    }

private:
    VkDevice mDevice;
    VkCommandBuffer mCmd;
};
namespace Colors
{
    inline constexpr float Orange[] = {1.f, 0.5f, 0.f, 1.f};
    inline constexpr float Cyan[] = {0.f, 0.8f, 0.8f, 1.f};
    inline constexpr float Green[] = {0.4f, 0.9f, 0.4f, 1.f};
    inline constexpr float Blue[] = {0.2f, 0.6f, 1.0f, 1.f};
    inline constexpr float Magenta[] = {0.9f, 0.4f, 0.8f, 1.f};
}
} // namespace Core::Renderer::Debug