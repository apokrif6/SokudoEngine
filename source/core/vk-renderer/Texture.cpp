#include <stb_image.h>

#include "core/vk-renderer/buffers/CommandBuffer.h"
#include "Texture.h"
#include "core/tools/Logger.h"

std::future<bool> Core::Renderer::Texture::loadTexture(Core::Renderer::VkRenderData& renderData,
                                                       VkTextureData& textureData, const std::string& textureFilename)
{
    const char* functionName = __FUNCTION__;
    return std::async(
        std::launch::async,
        [&renderData, &textureData, &textureFilename, &functionName]()
        {
            int texWidth;
            int texHeight;
            int numberOfChannels;

            unsigned char* texData =
                stbi_load(textureFilename.c_str(), &texWidth, &texHeight, &numberOfChannels, STBI_rgb_alpha);

            if (!texData)
            {
                perror("Error");
                Logger::log(1, "%s error: could not load file '%s', because of '%s'\n", "TOREMOVE",
                            textureFilename.c_str(), stbi_failure_reason());
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
            imageInfo.format = VK_FORMAT_R8G8B8A8_SRGB;
            imageInfo.tiling = VK_IMAGE_TILING_LINEAR;
            imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
            imageInfo.usage = VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
            imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
            imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;

            VmaAllocationCreateInfo imageAllocInfo{};
            imageAllocInfo.usage = VMA_MEMORY_USAGE_CPU_ONLY;

            if (vmaCreateImage(renderData.rdAllocator, &imageInfo, &imageAllocInfo, &textureData.texTextureImage,
                               &textureData.texTextureImageAlloc, nullptr) != VK_SUCCESS)
            {
                Logger::log(1, "%s error: could not allocate texture image via VMA\n", "TOREMOVE");
                return false;
            }

            VkBufferCreateInfo stagingBufferInfo{};
            stagingBufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
            stagingBufferInfo.size = imageSize;
            stagingBufferInfo.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;

            VkBuffer stagingBuffer;
            VmaAllocation stagingBufferAlloc;

            VmaAllocationCreateInfo stagingAllocInfo{};
            stagingAllocInfo.usage = VMA_MEMORY_USAGE_CPU_ONLY;

            if (vmaCreateBuffer(renderData.rdAllocator, &stagingBufferInfo, &stagingAllocInfo, &stagingBuffer,
                                &stagingBufferAlloc, nullptr) != VK_SUCCESS)
            {
                Logger::log(1, "%s error: could not allocate texture staging buffer via VMA\n", "TOREMOVE");
                return false;
            }

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
            stagingBufferTransferBarrier.image = textureData.texTextureImage;
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
            stagingBufferShaderBarrier.image = textureData.texTextureImage;
            stagingBufferShaderBarrier.subresourceRange = stagingBufferRange;
            stagingBufferShaderBarrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
            stagingBufferShaderBarrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

            VkCommandBuffer stagingCommandBuffer;

            if (!Core::Renderer::CommandBuffer::init(renderData, stagingCommandBuffer))
            {
                Logger::log(1, "%s error: could not create texture upload command buffers\n", "TOREMOVE");
                return false;
            }

            if (vkResetCommandBuffer(stagingCommandBuffer, 0) != VK_SUCCESS)
            {
                Logger::log(1, "%s error: failed to reset staging command buffer\n", "TOREMOVE");
                return false;
            }

            VkCommandBufferBeginInfo cmdBeginInfo{};
            cmdBeginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
            cmdBeginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

            if (vkBeginCommandBuffer(stagingCommandBuffer, &cmdBeginInfo) != VK_SUCCESS)
            {
                Logger::log(1, "%s error: failed to begin staging command buffer\n", "TOREMOVE");
                return false;
            }

            vkCmdPipelineBarrier(stagingCommandBuffer, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
                                 VK_PIPELINE_STAGE_TRANSFER_BIT, 0, 0, nullptr, 0, nullptr, 1,
                                 &stagingBufferTransferBarrier);
            vkCmdCopyBufferToImage(stagingCommandBuffer, stagingBuffer, textureData.texTextureImage,
                                   VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &stagingBufferCopy);
            vkCmdPipelineBarrier(stagingCommandBuffer, VK_PIPELINE_STAGE_TRANSFER_BIT,
                                 VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 0, 0, nullptr, 0, nullptr, 1,
                                 &stagingBufferShaderBarrier);

            if (vkEndCommandBuffer(stagingCommandBuffer) != VK_SUCCESS)
            {
                Logger::log(1, "%s error: failed to end staging command buffer\n", "TOREMOVE");
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
                Logger::log(1, "%s error: failed to create staging buffer fence\n", "TOREMOVE");
                return false;
            }

            if (vkResetFences(renderData.rdVkbDevice.device, 1, &stagingBufferFence) != VK_SUCCESS)
            {
                Logger::log(1, "%s error: staging buffer fence reset failed\n", "TOREMOVE");
                return false;
            }

            if (vkQueueSubmit(renderData.rdGraphicsQueue, 1, &submitInfo, stagingBufferFence) != VK_SUCCESS)
            {
                Logger::log(1, "%s error: failed to submit staging buffer copy command buffer\n", "TOREMOVE");
                return false;
            }

            if (vkWaitForFences(renderData.rdVkbDevice.device, 1, &stagingBufferFence, VK_TRUE, UINT64_MAX) !=
                VK_SUCCESS)
            {
                Logger::log(1, "%s error: waiting for staging buffer copy fence failed\n", "TOREMOVE");
                return false;
            }

            vkDestroyFence(renderData.rdVkbDevice.device, stagingBufferFence, nullptr);
            Core::Renderer::CommandBuffer::cleanup(renderData, stagingCommandBuffer);
            vmaDestroyBuffer(renderData.rdAllocator, stagingBuffer, stagingBufferAlloc);

            /* image view and sampler */
            VkImageViewCreateInfo texViewInfo{};
            texViewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
            texViewInfo.image = textureData.texTextureImage;
            texViewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
            texViewInfo.format = VK_FORMAT_R8G8B8A8_SRGB;
            texViewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
            texViewInfo.subresourceRange.baseMipLevel = 0;
            texViewInfo.subresourceRange.levelCount = 1;
            texViewInfo.subresourceRange.baseArrayLayer = 0;
            texViewInfo.subresourceRange.layerCount = 1;

            if (vkCreateImageView(renderData.rdVkbDevice.device, &texViewInfo, nullptr,
                                  &textureData.texTextureImageView) != VK_SUCCESS)
            {
                Logger::log(1, "%s error: could not create image view for texture\n", "TOREMOVE");
                return false;
            }

            VkSamplerCreateInfo texSamplerInfo{};
            texSamplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
            texSamplerInfo.magFilter = VK_FILTER_LINEAR;
            texSamplerInfo.minFilter = VK_FILTER_LINEAR;
            texSamplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
            texSamplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
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
                                &textureData.texTextureSampler) != VK_SUCCESS)
            {
                Logger::log(1, "%s error: could not create sampler for texture\n", "TOREMOVE");
                return false;
            }

            VkDescriptorSetLayoutBinding textureBind{};
            textureBind.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
            textureBind.binding = 0;
            textureBind.descriptorCount = 1;
            textureBind.pImmutableSamplers = nullptr;
            textureBind.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

            VkDescriptorSetLayoutCreateInfo textureCreateInfo{};
            textureCreateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
            textureCreateInfo.bindingCount = 1;
            textureCreateInfo.pBindings = &textureBind;

            if (vkCreateDescriptorSetLayout(renderData.rdVkbDevice.device, &textureCreateInfo, nullptr,
                                            &textureData.texTextureDescriptorLayout) != VK_SUCCESS)
            {
                Logger::log(1, "%s error: could not create descriptor set layout\n", "TOREMOVE");
                return false;
            }

            VkDescriptorPoolSize poolSize{};
            poolSize.type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
            poolSize.descriptorCount = 1;

            VkDescriptorPoolCreateInfo descriptorPool{};
            descriptorPool.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
            descriptorPool.poolSizeCount = 1;
            descriptorPool.pPoolSizes = &poolSize;
            descriptorPool.maxSets = 16;

            if (vkCreateDescriptorPool(renderData.rdVkbDevice.device, &descriptorPool, nullptr,
                                       &textureData.texTextureDescriptorPool) != VK_SUCCESS)
            {
                Logger::log(1, "%s error: could not create descriptor pool\n", "TOREMOVE");
                return false;
            }

            VkDescriptorSetAllocateInfo descriptorAllocateInfo{};
            descriptorAllocateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
            descriptorAllocateInfo.descriptorPool = textureData.texTextureDescriptorPool;
            descriptorAllocateInfo.descriptorSetCount = 1;
            descriptorAllocateInfo.pSetLayouts = &textureData.texTextureDescriptorLayout;

            if (vkAllocateDescriptorSets(renderData.rdVkbDevice.device, &descriptorAllocateInfo,
                                         &textureData.texTextureDescriptorSet) != VK_SUCCESS)
            {
                Logger::log(1, "%s error: could not allocate descriptor set\n", "TOREMOVE");
                return false;
            }

            VkDescriptorImageInfo descriptorImageInfo{};
            descriptorImageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
            descriptorImageInfo.imageView = textureData.texTextureImageView;
            descriptorImageInfo.sampler = textureData.texTextureSampler;

            VkWriteDescriptorSet writeDescriptorSet{};
            writeDescriptorSet.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
            writeDescriptorSet.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
            writeDescriptorSet.dstSet = textureData.texTextureDescriptorSet;
            writeDescriptorSet.dstBinding = 0;
            writeDescriptorSet.descriptorCount = 1;
            writeDescriptorSet.pImageInfo = &descriptorImageInfo;

            textureData.texName = textureFilename;

            vkUpdateDescriptorSets(renderData.rdVkbDevice.device, 1, &writeDescriptorSet, 0, nullptr);

            Logger::log(1, "%s: texture '%s' loaded (%dx%d, %d channels)\n", "TOREMOVE", textureFilename.c_str(),
                        texWidth, texHeight, numberOfChannels);
            return true;
        });
}

void Core::Renderer::Texture::cleanup(Core::Renderer::VkRenderData& renderData, VkTextureData& textureData)
{
    vkDestroyDescriptorPool(renderData.rdVkbDevice.device, textureData.texTextureDescriptorPool, nullptr);
    vkDestroyDescriptorSetLayout(renderData.rdVkbDevice.device, textureData.texTextureDescriptorLayout, nullptr);
    vkDestroySampler(renderData.rdVkbDevice.device, textureData.texTextureSampler, nullptr);
    vkDestroyImageView(renderData.rdVkbDevice.device, textureData.texTextureImageView, nullptr);
    vmaDestroyImage(renderData.rdAllocator, textureData.texTextureImage, textureData.texTextureImageAlloc);
}