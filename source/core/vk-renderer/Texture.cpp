#include "Texture.h"

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>
#include "core/vk-renderer/buffers/CommandBuffer.h"
#include "core/tools/Logger.h"

constexpr std::string_view textureFolderPath = "assets/textures/";

std::future<bool> Core::Renderer::Texture::loadTexture(VkRenderData& renderData,
                                                       VkTextureData& textureData, const std::string& textureFilename,
                                                       VkFormat format)
{
    return std::async(
        std::launch::async,
        [&renderData, &textureData, &textureFilename, format]
        {
            const std::string texturePath = textureFolderPath.data() + textureFilename;

            int texWidth;
            int texHeight;
            int numberOfChannels;

            unsigned char* texData =
                stbi_load(texturePath.c_str(), &texWidth, &texHeight, &numberOfChannels, STBI_rgb_alpha);

            if (!texData)
            {
                perror("Error");
                Logger::log(1, "Could not load file '%s', because of '%s'\n", texturePath.c_str(),
                            stbi_failure_reason());
                stbi_image_free(texData);
                return false;
            }

            VkDeviceSize imageSize = texWidth * texHeight * 4;

            VkImageCreateInfo imageInfo{};
            imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
            imageInfo.imageType = VK_IMAGE_TYPE_2D;
            imageInfo.extent.width = static_cast<uint32_t>(texWidth);
            imageInfo.extent.height = static_cast<uint32_t>(texHeight);
            imageInfo.extent.depth = 1;
            imageInfo.mipLevels = 1;
            imageInfo.arrayLayers = 1;
            imageInfo.format = format;
            imageInfo.tiling = VK_IMAGE_TILING_LINEAR;
            imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
            imageInfo.usage = VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
            imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
            imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;

            VmaAllocationCreateInfo imageAllocInfo{};
            imageAllocInfo.usage = VMA_MEMORY_USAGE_CPU_ONLY;

            if (vmaCreateImage(renderData.rdAllocator, &imageInfo, &imageAllocInfo, &textureData.image,
                               &textureData.imageAlloc, nullptr) != VK_SUCCESS)
            {
                Logger::log(1, "Could not allocate texture image via VMA\n");
                return false;
            }

            VkBufferCreateInfo stagingBufferInfo{};
            stagingBufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
            stagingBufferInfo.size = imageSize;
            stagingBufferInfo.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;

            VkBuffer stagingBuffer;
            VmaAllocation stagingBufferAlloc;

            VmaAllocationCreateInfo stagingAllocCreteInfo{};
            stagingAllocCreteInfo.usage = VMA_MEMORY_USAGE_CPU_ONLY;

            VmaAllocationInfo stagingAllocInfo{};

            if (vmaCreateBuffer(renderData.rdAllocator, &stagingBufferInfo, &stagingAllocCreteInfo, &stagingBuffer,
                                &stagingBufferAlloc, &stagingAllocInfo) != VK_SUCCESS)
            {
                Logger::log(1, "Could not allocate texture staging buffer via VMA\n");
                return false;
            }

            vmaSetAllocationName(renderData.rdAllocator, stagingBufferAlloc, "Texture Staging Buffer");

            void* data;
            vmaMapMemory(renderData.rdAllocator, stagingBufferAlloc, &data);
            std::memcpy(data, texData, static_cast<uint32_t>(imageSize));
            vmaUnmapMemory(renderData.rdAllocator, stagingBufferAlloc);

            stbi_image_free(texData);

            VkImageSubresourceRange stagingBufferRange{};
            stagingBufferRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
            stagingBufferRange.baseMipLevel = 0;
            stagingBufferRange.levelCount = 1;
            stagingBufferRange.baseArrayLayer = 0;
            stagingBufferRange.layerCount = 1;

            /* 1st barrier, undefined to transfer optimal */
            VkImageMemoryBarrier stagingBufferTransferBarrier{};
            stagingBufferTransferBarrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
            stagingBufferTransferBarrier.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
            stagingBufferTransferBarrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
            stagingBufferTransferBarrier.image = textureData.image;
            stagingBufferTransferBarrier.subresourceRange = stagingBufferRange;
            stagingBufferTransferBarrier.srcAccessMask = 0;
            stagingBufferTransferBarrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;

            VkExtent3D textureExtent{};
            textureExtent.width = static_cast<uint32_t>(texWidth);
            textureExtent.height = static_cast<uint32_t>(texHeight);
            textureExtent.depth = 1;

            VkBufferImageCopy stagingBufferCopy{};
            stagingBufferCopy.bufferOffset = 0;
            stagingBufferCopy.bufferRowLength = 0;
            stagingBufferCopy.bufferImageHeight = 0;
            stagingBufferCopy.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
            stagingBufferCopy.imageSubresource.mipLevel = 0;
            stagingBufferCopy.imageSubresource.baseArrayLayer = 0;
            stagingBufferCopy.imageSubresource.layerCount = 1;
            stagingBufferCopy.imageExtent = textureExtent;

            /* 2nd barrier, transfer optimal to shader optimal */
            VkImageMemoryBarrier stagingBufferShaderBarrier{};
            stagingBufferShaderBarrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
            stagingBufferShaderBarrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
            stagingBufferShaderBarrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
            stagingBufferShaderBarrier.image = textureData.image;
            stagingBufferShaderBarrier.subresourceRange = stagingBufferRange;
            stagingBufferShaderBarrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
            stagingBufferShaderBarrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

            VkCommandBuffer stagingCommandBuffer;

            if (!Core::Renderer::CommandBuffer::init(renderData, stagingCommandBuffer))
            {
                Logger::log(1, "Could not create texture upload command buffers\n");
                return false;
            }

            if (vkResetCommandBuffer(stagingCommandBuffer, 0) != VK_SUCCESS)
            {
                Logger::log(1, "Failed to reset staging command buffer\n");
                return false;
            }

            VkCommandBufferBeginInfo cmdBeginInfo{};
            cmdBeginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
            cmdBeginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

            if (vkBeginCommandBuffer(stagingCommandBuffer, &cmdBeginInfo) != VK_SUCCESS)
            {
                Logger::log(1, "%s error: failed to begin staging command buffer\n");
                return false;
            }

            vkCmdPipelineBarrier(stagingCommandBuffer, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
                                 VK_PIPELINE_STAGE_TRANSFER_BIT, 0, 0, nullptr, 0, nullptr, 1,
                                 &stagingBufferTransferBarrier);
            vkCmdCopyBufferToImage(stagingCommandBuffer, stagingBuffer, textureData.image,
                                   VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &stagingBufferCopy);
            vkCmdPipelineBarrier(stagingCommandBuffer, VK_PIPELINE_STAGE_TRANSFER_BIT,
                                 VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 0, 0, nullptr, 0, nullptr, 1,
                                 &stagingBufferShaderBarrier);

            if (vkEndCommandBuffer(stagingCommandBuffer) != VK_SUCCESS)
            {
                Logger::log(1, "Failed to end staging command buffer\n");
                return false;
            }

            VkSubmitInfo submitInfo{};
            submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
            submitInfo.pWaitDstStageMask = nullptr;
            submitInfo.waitSemaphoreCount = 0;
            submitInfo.pWaitSemaphores = nullptr;
            submitInfo.signalSemaphoreCount = 0;
            submitInfo.pSignalSemaphores = nullptr;
            submitInfo.commandBufferCount = 1;
            submitInfo.pCommandBuffers = &stagingCommandBuffer;

            VkFence stagingBufferFence;

            VkFenceCreateInfo fenceInfo{};
            fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
            fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

            if (vkCreateFence(renderData.rdVkbDevice.device, &fenceInfo, nullptr, &stagingBufferFence) != VK_SUCCESS)
            {
                Logger::log(1, "Failed to create staging buffer fence\n");
                return false;
            }

            if (vkResetFences(renderData.rdVkbDevice.device, 1, &stagingBufferFence) != VK_SUCCESS)
            {
                Logger::log(1, "Staging buffer fence reset failed\n");
                return false;
            }

            if (vkQueueSubmit(renderData.rdGraphicsQueue, 1, &submitInfo, stagingBufferFence) != VK_SUCCESS)
            {
                Logger::log(1, "Failed to submit staging buffer copy command buffer\n");
                return false;
            }

            if (vkWaitForFences(renderData.rdVkbDevice.device, 1, &stagingBufferFence, VK_TRUE, UINT64_MAX) !=
                VK_SUCCESS)
            {
                Logger::log(1, "Error: waiting for staging buffer copy fence failed\n");
                return false;
            }

            vkDestroyFence(renderData.rdVkbDevice.device, stagingBufferFence, nullptr);
            Core::Renderer::CommandBuffer::cleanup(renderData, stagingCommandBuffer);
            vmaDestroyBuffer(renderData.rdAllocator, stagingBuffer, stagingBufferAlloc);

            /* image view and sampler */
            VkImageViewCreateInfo texViewInfo{};
            texViewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
            texViewInfo.image = textureData.image;
            texViewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
            texViewInfo.format = format;
            texViewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
            texViewInfo.subresourceRange.baseMipLevel = 0;
            texViewInfo.subresourceRange.levelCount = 1;
            texViewInfo.subresourceRange.baseArrayLayer = 0;
            texViewInfo.subresourceRange.layerCount = 1;

            if (vkCreateImageView(renderData.rdVkbDevice.device, &texViewInfo, nullptr,
                                  &textureData.imageView) != VK_SUCCESS)
            {
                Logger::log(1, "Could not create image view for texture\n");
                return false;
            }

            VkSamplerCreateInfo texSamplerInfo{};
            texSamplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
            texSamplerInfo.magFilter = VK_FILTER_LINEAR;
            texSamplerInfo.minFilter = VK_FILTER_LINEAR;
            texSamplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
            texSamplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
            texSamplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
            texSamplerInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
            texSamplerInfo.unnormalizedCoordinates = VK_FALSE;
            texSamplerInfo.compareEnable = VK_FALSE;
            texSamplerInfo.compareOp = VK_COMPARE_OP_ALWAYS;
            texSamplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
            texSamplerInfo.mipLodBias = 0.0f;
            texSamplerInfo.minLod = 0.0f;
            texSamplerInfo.maxLod = 0.0f;
            texSamplerInfo.anisotropyEnable = VK_FALSE;
            texSamplerInfo.maxAnisotropy = 1.0f;

            if (vkCreateSampler(renderData.rdVkbDevice.device, &texSamplerInfo, nullptr,
                                &textureData.sampler) != VK_SUCCESS)
            {
                Logger::log(1, "Could not create sampler for texture\n");
                return false;
            }

            textureData.name = texturePath;

            Logger::log(1, "Texture '%s' loaded (%dx%d, %d channels)\n", texturePath.c_str(), texWidth, texHeight,
                        numberOfChannels);
            return true;
        });
}

std::future<bool> Core::Renderer::Texture::loadHDRTexture(VkRenderData& renderData, VkTextureData& textureData,
    const std::string& textureFilename, VkFormat format)
{
    return std::async(
        std::launch::async,
        [&renderData, &textureData, textureFilename, format]
        {
            const std::string texturePath = textureFolderPath.data() + textureFilename;

            stbi_set_flip_vertically_on_load(true);

            int width, height, channels;
            float* hdrData = stbi_loadf(texturePath.c_str(), &width, &height, &channels, STBI_rgb_alpha);

            if (!hdrData)
            {
                Logger::log(1, "%s error: failed to load HDR texture %s", __FUNCTION__, texturePath.c_str());
                return false;
            }

            VkDeviceSize imageSize = width * height * STBI_rgb_alpha * sizeof(float);

            VkBuffer stagingBuffer;
            VmaAllocation stagingAlloc;

            VkBufferCreateInfo bufferInfo{};
            bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
            bufferInfo.size = imageSize;
            bufferInfo.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;

            VmaAllocationCreateInfo allocInfo{};
            allocInfo.usage = VMA_MEMORY_USAGE_CPU_ONLY;

            if (vmaCreateBuffer(renderData.rdAllocator, &bufferInfo, &allocInfo,
                                &stagingBuffer, &stagingAlloc, nullptr) != VK_SUCCESS)
            {
                Logger::log(1, "%s error: failed to create HDR staging buffer", __FUNCTION__);
                stbi_image_free(hdrData);
                return false;
            }

            vmaSetAllocationName(renderData.rdAllocator, stagingAlloc, "HDR Texture Staging Buffer");

            void* mapped;
            vmaMapMemory(renderData.rdAllocator, stagingAlloc, &mapped);
            memcpy(mapped, hdrData, imageSize);
            vmaUnmapMemory(renderData.rdAllocator, stagingAlloc);

            stbi_image_free(hdrData);

            VkImageCreateInfo imageInfo{};
            imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
            imageInfo.imageType = VK_IMAGE_TYPE_2D;
            imageInfo.format = VK_FORMAT_R32G32B32A32_SFLOAT;
            imageInfo.extent = { static_cast<uint32_t>(width), static_cast<uint32_t>(height), 1 };
            imageInfo.mipLevels = 1;
            imageInfo.arrayLayers = 1;
            imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
            imageInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
            imageInfo.usage =
                VK_IMAGE_USAGE_TRANSFER_DST_BIT |
                VK_IMAGE_USAGE_SAMPLED_BIT;
            imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
            imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

            VmaAllocationCreateInfo imageAllocInfo{};
            imageAllocInfo.usage = VMA_MEMORY_USAGE_GPU_ONLY;

            if (vmaCreateImage(renderData.rdAllocator, &imageInfo, &imageAllocInfo,
                               &textureData.image, &textureData.imageAlloc, nullptr) != VK_SUCCESS)
            {
                Logger::log(1, "%s error: failed to create HDR image", __FUNCTION__);
                vmaDestroyBuffer(renderData.rdAllocator, stagingBuffer, stagingAlloc);
                return false;
            }

            VkCommandBuffer stagingCommandBuffer;
            if (!CommandBuffer::init(renderData, stagingCommandBuffer))

                return false;

            VkCommandBufferBeginInfo beginInfo{};
            beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
            beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

            vkBeginCommandBuffer(stagingCommandBuffer, &beginInfo);

            VkImageMemoryBarrier barrier{};
            barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
            barrier.image = textureData.image;
            barrier.srcAccessMask = 0;
            barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
            barrier.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
            barrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
            barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
            barrier.subresourceRange.baseMipLevel = 0;
            barrier.subresourceRange.levelCount = 1;
            barrier.subresourceRange.baseArrayLayer = 0;
            barrier.subresourceRange.layerCount = 1;

            vkCmdPipelineBarrier(stagingCommandBuffer, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT,
                0, 0, nullptr, 0, nullptr,
                1, &barrier);

            VkBufferImageCopy copy{};
            copy.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
            copy.imageSubresource.layerCount = 1;
            copy.imageExtent = { static_cast<uint32_t>(width), static_cast<uint32_t>(height), 1 };

            vkCmdCopyBufferToImage(stagingCommandBuffer, stagingBuffer, textureData.image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &copy);

            barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
            barrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
            barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
            barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

            vkCmdPipelineBarrier(stagingCommandBuffer, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
                0, 0, nullptr, 0, nullptr,
                1, &barrier);

            vkEndCommandBuffer(stagingCommandBuffer);

            VkSubmitInfo submitInfo{};
            submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
            submitInfo.commandBufferCount = 1;
            submitInfo.pCommandBuffers = &stagingCommandBuffer;

            vkQueueSubmit(renderData.rdGraphicsQueue, 1, &submitInfo, VK_NULL_HANDLE);
            vkQueueWaitIdle(renderData.rdGraphicsQueue);

            vkFreeCommandBuffers(renderData.rdVkbDevice.device, renderData.rdCommandPool, 1, &stagingCommandBuffer);
            vmaDestroyBuffer(renderData.rdAllocator, stagingBuffer, stagingAlloc);

            VkImageViewCreateInfo viewInfo{};
            viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
            viewInfo.image = textureData.image;
            viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
            viewInfo.format = VK_FORMAT_R32G32B32A32_SFLOAT;
            viewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
            viewInfo.subresourceRange.baseMipLevel = 0;
            viewInfo.subresourceRange.levelCount = 1;
            viewInfo.subresourceRange.baseArrayLayer = 0;
            viewInfo.subresourceRange.layerCount = 1;

            if (vkCreateImageView(renderData.rdVkbDevice.device, &viewInfo, nullptr, &textureData.imageView) != VK_SUCCESS)
            {
                Logger::log(1, "%s error: failed to create HDR image view", __FUNCTION__);
                return false;
            }

            VkSamplerCreateInfo samplerInfo{};
            samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
            samplerInfo.magFilter = VK_FILTER_LINEAR;
            samplerInfo.minFilter = VK_FILTER_LINEAR;
            samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
            samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
            samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
            samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
            samplerInfo.minLod = 0.f;
            samplerInfo.maxLod = 1.f;

            if (vkCreateSampler(renderData.rdVkbDevice.device, &samplerInfo, nullptr, &textureData.sampler) != VK_SUCCESS)
            {
                Logger::log(1, "%s error: failed to create HDR sampler", __FUNCTION__);
                return false;
            }

            textureData.name = texturePath;

            Logger::log(1, "HDR Texture '%s' loaded (%dx%d, %d channels)\n", texturePath.c_str(), width, height,
                        channels);
            return true;
        });
}

void Core::Renderer::Texture::cleanup(Core::Renderer::VkRenderData& renderData, VkTextureData& textureData)
{
    vkDestroySampler(renderData.rdVkbDevice.device, textureData.sampler, nullptr);
    vkDestroyImageView(renderData.rdVkbDevice.device, textureData.imageView, nullptr);
    vmaDestroyImage(renderData.rdAllocator, textureData.image, textureData.imageAlloc);
}
