#include "ViewportTarget.h"
#include "core/tools/Logger.h"
#include <imgui_impl_vulkan.h>

bool Core::Renderer::ViewportTarget::init(VkRenderData& renderData, glm::int2 size)
{
    renderData.rdViewportTarget.size = size;

    if (!createImage(renderData))
        return false;
    if (!createImageView(renderData))
        return false;
    if (!createFramebuffer(renderData))
        return false;
    if (!createDescriptor(renderData))
        return false;
    return true;
}

bool Core::Renderer::ViewportTarget::createImage(VkRenderData& renderData)
{
    VkExtent2D extent = {static_cast<uint32_t>(renderData.rdViewportTarget.size.x),
                         static_cast<uint32_t>(renderData.rdViewportTarget.size.y)};

    VkImageCreateInfo imageInfo{};
    imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    imageInfo.imageType = VK_IMAGE_TYPE_2D;
    imageInfo.extent = {extent.width, extent.height, 1};
    imageInfo.mipLevels = 1;
    imageInfo.arrayLayers = 1;
    imageInfo.format = VK_FORMAT_B8G8R8A8_UNORM;
    imageInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
    imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    imageInfo.usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
    imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;

    VmaAllocationCreateInfo allocInfo{};
    allocInfo.usage = VMA_MEMORY_USAGE_GPU_ONLY;

    if (vmaCreateImage(renderData.rdAllocator, &imageInfo, &allocInfo, &renderData.rdViewportTarget.image,
                       &renderData.rdViewportTarget.imageAlloc, nullptr) != VK_SUCCESS)
    {
        Logger::log(1, "%s error: could not create viewport image\n", __FUNCTION__);
        return false;
    }
    return true;
}

bool Core::Renderer::ViewportTarget::createImageView(VkRenderData& renderData)
{
    VkImageViewCreateInfo viewInfo{};
    viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    viewInfo.image = renderData.rdViewportTarget.image;
    viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
    viewInfo.format = VK_FORMAT_B8G8R8A8_UNORM;
    viewInfo.subresourceRange = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1};

    if (vkCreateImageView(renderData.rdVkbDevice.device, &viewInfo, nullptr, &renderData.rdViewportTarget.imageView) !=
        VK_SUCCESS)
    {
        Logger::log(1, "%s error: could not create viewport image view\n", __FUNCTION__);
        return false;
    }
    return true;
}

bool Core::Renderer::ViewportTarget::createFramebuffer(VkRenderData& renderData)
{
    VkImageView attachments[] = {renderData.rdViewportTarget.imageView, renderData.rdDepthImageView};

    VkFramebufferCreateInfo fbInfo{};
    fbInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
    fbInfo.renderPass = renderData.rdViewportTarget.renderpass;
    fbInfo.attachmentCount = 2;
    fbInfo.pAttachments = attachments;
    fbInfo.width = static_cast<uint32_t>(renderData.rdViewportTarget.size.x);
    fbInfo.height = static_cast<uint32_t>(renderData.rdViewportTarget.size.y);
    fbInfo.layers = 1;

    if (vkCreateFramebuffer(renderData.rdVkbDevice.device, &fbInfo, nullptr,
                            &renderData.rdViewportTarget.framebuffer) != VK_SUCCESS)
    {
        Logger::log(1, "%s error: could not create viewport framebuffer\n", __FUNCTION__);
        return false;
    }
    return true;
}

bool Core::Renderer::ViewportTarget::createDescriptor(VkRenderData& renderData)
{
    VkSamplerCreateInfo samplerInfo{};
    samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
    samplerInfo.magFilter = VK_FILTER_LINEAR;
    samplerInfo.minFilter = VK_FILTER_LINEAR;
    samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
    samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
    samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
    samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;

    if (vkCreateSampler(renderData.rdVkbDevice.device, &samplerInfo, nullptr, &mSampler) != VK_SUCCESS)
    {
        Logger::log(1, "%s error: could not create viewport sampler\n", __FUNCTION__);
        return false;
    }

    renderData.rdViewportTarget.descriptorSet = ImGui_ImplVulkan_AddTexture(
        mSampler, renderData.rdViewportTarget.imageView, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

    return true;
}

void Core::Renderer::ViewportTarget::cleanup(VkRenderData& renderData)
{
    auto dev = renderData.rdVkbDevice.device;
    vkDestroySampler(dev, mSampler, nullptr);
    vkDestroyFramebuffer(dev, renderData.rdViewportTarget.framebuffer, nullptr);
    vkDestroyImageView(dev, renderData.rdViewportTarget.imageView, nullptr);
    vmaDestroyImage(renderData.rdAllocator, renderData.rdViewportTarget.image, renderData.rdViewportTarget.imageAlloc);
}
