#include "SyncObjects.h"
#include "core/tools/Logger.h"

bool Core::Renderer::SyncObjects::init(Core::Renderer::VkRenderData& renderData)
{
    VkFenceCreateInfo fenceInfo{};
    fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

    VkSemaphoreCreateInfo semaphoreInfo{};
    semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

    if (vkCreateSemaphore(renderData.rdVkbDevice.device, &semaphoreInfo, nullptr, &renderData.rdPresentSemaphore) !=
            VK_SUCCESS ||
        vkCreateSemaphore(renderData.rdVkbDevice.device, &semaphoreInfo, nullptr, &renderData.rdRenderSemaphore) !=
            VK_SUCCESS ||
        vkCreateFence(renderData.rdVkbDevice.device, &fenceInfo, nullptr, &renderData.rdRenderFence) != VK_SUCCESS)
    {
        Logger::log(1, "%s error: failed to init sync objects\n", __FUNCTION__);
        return false;
    }
    return true;
}

void Core::Renderer::SyncObjects::cleanup(Core::Renderer::VkRenderData& renderData)
{
    vkDestroySemaphore(renderData.rdVkbDevice.device, renderData.rdPresentSemaphore, nullptr);
    vkDestroySemaphore(renderData.rdVkbDevice.device, renderData.rdRenderSemaphore, nullptr);
    vkDestroyFence(renderData.rdVkbDevice.device, renderData.rdRenderFence, nullptr);
}
