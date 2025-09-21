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

        auto function = (PFN_vkSetDebugUtilsObjectNameEXT)vkGetDeviceProcAddr(device, "vkSetDebugUtilsObjectNameEXT");
        if (function)
        {
            function(device, &nameInfo);
        }
    }
}