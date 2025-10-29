#include "Cubemap.h"
#include "core/tools/Logger.h"
#include "stb_image.h"
#include "core/vk-renderer/buffers/CommandBuffer.h"

std::future<bool> Core::Renderer::Cubemap::loadCubemap(Core::Renderer::VkRenderData& renderData,
                                                       Core::Renderer::VkCubemapData& cubemapData,
                                                       const std::vector<std::string>& faces)
{
    return std::async(
            std::launch::async,
            [&renderData, &cubemapData, faces]
            {
                std::vector<unsigned char*> imageData;
                imageData.reserve(6);

                std::vector<int> widths;
                widths.reserve(6);

                std::vector<int> heights;
                heights.reserve(6);

                std::vector<int> channels;
                channels.reserve(6);

                size_t totalSize = 0;

                for (auto face : faces)
                {
                    int weight;
                    int height;
                    int numberOfChannels;
                    unsigned char* data = stbi_load(face.c_str(), &weight, &height, &numberOfChannels, STBI_rgb_alpha);

                    if (!data)
                    {
                        perror("Error");
                        Logger::log(1, "%s error: could not load cubemap face %s", __FUNCTION__, face.c_str());
                        for (auto* image : imageData)
                        {
                            stbi_image_free(image);
                        }
                        return false;
                    }

                    imageData.push_back(data);
                    widths.push_back(weight);
                    heights.push_back(height);
                    channels.push_back(4);

                    totalSize += weight * height * 4;
                }

                VkBufferCreateInfo stagingBufferInfo{};
                stagingBufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
                stagingBufferInfo.size = totalSize;
                stagingBufferInfo.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;

                VmaAllocationCreateInfo stagingAllocInfo{};
                stagingAllocInfo.usage = VMA_MEMORY_USAGE_CPU_ONLY;

                VkBuffer stagingBuffer;
                VmaAllocation stagingBufferAllocation;

                if (vmaCreateBuffer(renderData.rdAllocator, &stagingBufferInfo, &stagingAllocInfo,
                                    &stagingBuffer, &stagingBufferAllocation, nullptr) != VK_SUCCESS)
                {
                    Logger::log(1, "%s error: could not create staging buffer", __FUNCTION__);
                    for (auto* image : imageData)
                    {
                        stbi_image_free(image);
                    }
                    return false;
                }

                vmaSetAllocationName(renderData.rdAllocator, stagingBufferAllocation, "Cubemap Staging Buffer");

                void* data;
                vmaMapMemory(renderData.rdAllocator, stagingBufferAllocation, &data);
                char* dataPtr = static_cast<char*>(data);
                size_t offset = 0;

                for (size_t i = 0; i < faces.size(); ++i)
                {
                    const size_t faceSize = widths[i] * heights[i] * 4;
                    memcpy(dataPtr + offset, imageData[i], faceSize);
                    offset += faceSize;
                    stbi_image_free(imageData[i]);
                }

                vmaUnmapMemory(renderData.rdAllocator, stagingBufferAllocation);

                VkImageCreateInfo imageInfo{};
                imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
                imageInfo.imageType = VK_IMAGE_TYPE_2D;
                imageInfo.format = VK_FORMAT_R8G8B8A8_UNORM;
                imageInfo.mipLevels = 1;
                imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
                imageInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
                imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
                imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
                imageInfo.extent = { static_cast<uint32_t>(widths[0]), static_cast<uint32_t>(heights[0]), 1 };
                imageInfo.usage = VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
                imageInfo.arrayLayers = 6;
                imageInfo.flags = VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT;

                VmaAllocationCreateInfo imageAllocInfo{};
                imageAllocInfo.usage = VMA_MEMORY_USAGE_GPU_ONLY;

                if (vmaCreateImage(renderData.rdAllocator, &imageInfo, &imageAllocInfo,
                                   &cubemapData.image, &cubemapData.imageAlloc, nullptr) != VK_SUCCESS)
                {
                    Logger::log(1, "%s error: could not create cubemap image", __FUNCTION__);
                    vmaDestroyBuffer(renderData.rdAllocator, stagingBuffer, stagingBufferAllocation);
                    return false;
                }

                VkCommandBuffer stagingCommandBuffer;

                if (!Core::Renderer::CommandBuffer::init(renderData, stagingCommandBuffer))
                {
                    Logger::log(1, "Could not create cubemap upload command buffers");
                    return false;
                }

                if (vkResetCommandBuffer(stagingCommandBuffer, 0) != VK_SUCCESS)
                {
                    Logger::log(1, "Failed to reset staging command buffer");
                    return false;
                }

                VkCommandBufferBeginInfo cmdBeginInfo{};
                cmdBeginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
                cmdBeginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

                if (vkBeginCommandBuffer(stagingCommandBuffer, &cmdBeginInfo) != VK_SUCCESS)
                {
                    Logger::log(1, "%s error: failed to begin staging command buffer", __FUNCTION__);
                    return false;
                }

                VkImageMemoryBarrier barrier{};
                barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
                barrier.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
                barrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
                barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
                barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
                barrier.image = cubemapData.image;
                barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
                barrier.subresourceRange.baseMipLevel = 0;
                barrier.subresourceRange.levelCount = 1;
                barrier.subresourceRange.baseArrayLayer = 0;
                barrier.subresourceRange.layerCount = 6;
                barrier.srcAccessMask = 0;
                barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;

                vkCmdPipelineBarrier(stagingCommandBuffer,
                                     VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT,
                                     0, 0, nullptr, 0, nullptr, 1, &barrier);

                offset = 0;
                std::vector<VkBufferImageCopy> regions;
                regions.reserve(6);

                for (uint32_t face = 0; face < 6; face++)
                {
                    VkBufferImageCopy region{};
                    region.bufferOffset = offset;
                    region.bufferRowLength = 0;
                    region.bufferImageHeight = 0;
                    region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
                    region.imageSubresource.mipLevel = 0;
                    region.imageSubresource.baseArrayLayer = face;
                    region.imageSubresource.layerCount = 1;
                    region.imageOffset = {0, 0, 0};
                    region.imageExtent = {static_cast<uint32_t>(widths[face]),
                                          static_cast<uint32_t>(heights[face]), 1};

                    regions.push_back(region);
                    offset += widths[face] * heights[face] * 4;
                }

                vkCmdCopyBufferToImage(stagingCommandBuffer, stagingBuffer, cubemapData.image,
                                       VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, static_cast<uint32_t>(regions.size()), regions.data());

                barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
                barrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
                barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
                barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

                vkCmdPipelineBarrier(stagingCommandBuffer,
                                     VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
                                     0, 0, nullptr, 0, nullptr, 1, &barrier);

                if (vkEndCommandBuffer(stagingCommandBuffer) != VK_SUCCESS)
                {
                    Logger::log(1, "Failed to end staging command buffer");
                    return false;
                }

                VkSubmitInfo submitInfo{};
                submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
                submitInfo.commandBufferCount = 1;
                submitInfo.pCommandBuffers = &stagingCommandBuffer;

                vkQueueSubmit(renderData.rdGraphicsQueue, 1, &submitInfo, VK_NULL_HANDLE);
                vkQueueWaitIdle(renderData.rdGraphicsQueue);

                vkFreeCommandBuffers(renderData.rdVkbDevice.device, renderData.rdCommandPool, 1, &stagingCommandBuffer);
                vmaDestroyBuffer(renderData.rdAllocator, stagingBuffer, stagingBufferAllocation);

                VkImageViewCreateInfo viewInfo{};
                viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
                viewInfo.image = cubemapData.image;
                viewInfo.viewType = VK_IMAGE_VIEW_TYPE_CUBE;
                viewInfo.format = VK_FORMAT_R8G8B8A8_UNORM;
                viewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
                viewInfo.subresourceRange.baseMipLevel = 0;
                viewInfo.subresourceRange.levelCount = 1;
                viewInfo.subresourceRange.baseArrayLayer = 0;
                viewInfo.subresourceRange.layerCount = 6;

                if (vkCreateImageView(renderData.rdVkbDevice.device, &viewInfo, nullptr, &cubemapData.imageView) != VK_SUCCESS)
                {
                    Logger::log(1, "%s error: failed to create cubemap image view", __FUNCTION__);
                    return false;
                }

                VkSamplerCreateInfo samplerInfo{};
                samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
                samplerInfo.magFilter = VK_FILTER_LINEAR;
                samplerInfo.minFilter = VK_FILTER_LINEAR;
                samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
                samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
                samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
                samplerInfo.anisotropyEnable = VK_TRUE;
                samplerInfo.maxAnisotropy = 16.0f;
                samplerInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
                samplerInfo.unnormalizedCoordinates = VK_FALSE;
                samplerInfo.compareEnable = VK_FALSE;
                samplerInfo.compareOp = VK_COMPARE_OP_ALWAYS;
                samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
                samplerInfo.mipLodBias = 0.0f;
                samplerInfo.minLod = 0.0f;
                samplerInfo.maxLod = 0.0f;

                if (vkCreateSampler(renderData.rdVkbDevice.device, &samplerInfo, nullptr, &cubemapData.sampler) != VK_SUCCESS)
                {
                    Logger::log(1, "%s error: failed to create cubemap sampler", __FUNCTION__);
                    return false;
                }

                VkDescriptorSetLayoutBinding samplerLayoutBinding{};
                samplerLayoutBinding.binding = 0;
                samplerLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
                samplerLayoutBinding.descriptorCount = 1;
                samplerLayoutBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

                VkDescriptorSetLayoutCreateInfo layoutInfo{};
                layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
                layoutInfo.bindingCount = 1;
                layoutInfo.pBindings = &samplerLayoutBinding;

                if (vkCreateDescriptorSetLayout(renderData.rdVkbDevice.device, &layoutInfo, nullptr,
                                                &cubemapData.descriptorSetLayout) != VK_SUCCESS)
                {
                    Logger::log(1, "%s error: failed to create descriptor set layout", __FUNCTION__);
                    return false;
                }

                VkDescriptorPoolSize poolSize{};
                poolSize.type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
                poolSize.descriptorCount = 1;

                VkDescriptorPoolCreateInfo poolInfo{};
                poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
                poolInfo.poolSizeCount = 1;
                poolInfo.pPoolSizes = &poolSize;
                poolInfo.maxSets = 1;

                if (vkCreateDescriptorPool(renderData.rdVkbDevice.device, &poolInfo, nullptr,
                                           &cubemapData.descriptorPool) != VK_SUCCESS)
                {
                    Logger::log(1, "%s error: failed to create descriptor pool", __FUNCTION__);
                    return false;
                }

                VkDescriptorSetAllocateInfo allocInfo2{};
                allocInfo2.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
                allocInfo2.descriptorPool = cubemapData.descriptorPool;
                allocInfo2.descriptorSetCount = 1;
                allocInfo2.pSetLayouts = &cubemapData.descriptorSetLayout;

                if (vkAllocateDescriptorSets(renderData.rdVkbDevice.device, &allocInfo2,
                                             &cubemapData.descriptorSet) != VK_SUCCESS)
                {
                    Logger::log(1, "%s error: failed to allocate descriptor set", __FUNCTION__);
                    return false;
                }

                VkDescriptorImageInfo imageInfo2{};
                imageInfo2.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
                imageInfo2.imageView = cubemapData.imageView;
                imageInfo2.sampler = cubemapData.sampler;

                VkWriteDescriptorSet descriptorWrite{};
                descriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
                descriptorWrite.dstSet = cubemapData.descriptorSet;
                descriptorWrite.dstBinding = 0;
                descriptorWrite.dstArrayElement = 0;
                descriptorWrite.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
                descriptorWrite.descriptorCount = 1;
                descriptorWrite.pImageInfo = &imageInfo2;

                vkUpdateDescriptorSets(renderData.rdVkbDevice.device, 1, &descriptorWrite, 0, nullptr);

                cubemapData.width = widths[0];
                cubemapData.height = heights[0];

                Logger::log(1, "%s: created cubemap", __FUNCTION__);

                return true;
            });
}

void Core::Renderer::Cubemap::cleanup(Core::Renderer::VkRenderData& renderData, Core::Renderer::VkCubemapData& cubemapData)
{
    vkDestroySampler(renderData.rdVkbDevice.device, cubemapData.sampler, nullptr);
    vkDestroyImageView(renderData.rdVkbDevice.device, cubemapData.imageView, nullptr);
    vmaDestroyImage(renderData.rdAllocator, cubemapData.image, cubemapData.imageAlloc);
    vkDestroyDescriptorSetLayout(renderData.rdVkbDevice.device, cubemapData.descriptorSetLayout, nullptr);
    vkDestroyDescriptorPool(renderData.rdVkbDevice.device, cubemapData.descriptorPool, nullptr);
}
