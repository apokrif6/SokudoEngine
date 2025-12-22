#include "Cubemap.h"
#include "core/tools/Logger.h"
#include "stb_image.h"
#include "core/vk-renderer/buffers/CommandBuffer.h"

std::future<bool> Core::Renderer::Cubemap::loadCubemap(VkRenderData& renderData,
                                                       VkCubemapData& cubemapData,
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

            if (vmaCreateBuffer(renderData.rdAllocator, &stagingBufferInfo, &stagingAllocInfo, &stagingBuffer,
                                &stagingBufferAllocation, nullptr) != VK_SUCCESS)
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
            imageInfo.extent = {static_cast<uint32_t>(widths[0]), static_cast<uint32_t>(heights[0]), 1};
            imageInfo.usage = VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
            imageInfo.arrayLayers = 6;
            imageInfo.flags = VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT;

            VmaAllocationCreateInfo imageAllocInfo{};
            imageAllocInfo.usage = VMA_MEMORY_USAGE_GPU_ONLY;

            if (vmaCreateImage(renderData.rdAllocator, &imageInfo, &imageAllocInfo, &cubemapData.image,
                               &cubemapData.imageAlloc, nullptr) != VK_SUCCESS)
            {
                Logger::log(1, "%s error: could not create cubemap image", __FUNCTION__);
                vmaDestroyBuffer(renderData.rdAllocator, stagingBuffer, stagingBufferAllocation);
                return false;
            }

            VkCommandBuffer stagingCommandBuffer;

            if (!CommandBuffer::init(renderData, stagingCommandBuffer))
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

            vkCmdPipelineBarrier(stagingCommandBuffer, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
                                 VK_PIPELINE_STAGE_TRANSFER_BIT, 0, 0, nullptr, 0, nullptr, 1, &barrier);

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
                region.imageExtent = {static_cast<uint32_t>(widths[face]), static_cast<uint32_t>(heights[face]), 1};

                regions.push_back(region);
                offset += widths[face] * heights[face] * 4;
            }

            vkCmdCopyBufferToImage(stagingCommandBuffer, stagingBuffer, cubemapData.image,
                                   VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, static_cast<uint32_t>(regions.size()),
                                   regions.data());

            barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
            barrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
            barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
            barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

            vkCmdPipelineBarrier(stagingCommandBuffer, VK_PIPELINE_STAGE_TRANSFER_BIT,
                                 VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 0, 0, nullptr, 0, nullptr, 1, &barrier);

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

            if (vkCreateImageView(renderData.rdVkbDevice.device, &viewInfo, nullptr, &cubemapData.imageView) !=
                VK_SUCCESS)
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

            if (vkCreateSampler(renderData.rdVkbDevice.device, &samplerInfo, nullptr, &cubemapData.sampler) !=
                VK_SUCCESS)
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

            if (vkAllocateDescriptorSets(renderData.rdVkbDevice.device, &allocInfo2, &cubemapData.descriptorSet) !=
                VK_SUCCESS)
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

bool Core::Renderer::Cubemap::loadHDRTexture(VkRenderData& renderData, VkHDRTextureData& texture, const std::string& path)
{
    stbi_set_flip_vertically_on_load(true);

    int width, height, channels;
    float* hdrData = stbi_loadf(path.c_str(), &width, &height, &channels, 3);

    if (!hdrData)
    {
        Logger::log(1, "%s error: failed to load HDR %s", __FUNCTION__, path.c_str());
        return false;
    }

    VkDeviceSize imageSize = width * height * 3 * sizeof(float);

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

    void* mapped;
    vmaMapMemory(renderData.rdAllocator, stagingAlloc, &mapped);
    memcpy(mapped, hdrData, imageSize);
    vmaUnmapMemory(renderData.rdAllocator, stagingAlloc);

    stbi_image_free(hdrData);

    VkImageCreateInfo imageInfo{};
    imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    imageInfo.imageType = VK_IMAGE_TYPE_2D;
    imageInfo.format = VK_FORMAT_R16G16B16A16_SFLOAT;
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
                       &texture.image, &texture.imageAlloc, nullptr) != VK_SUCCESS)
    {
        Logger::log(1, "%s error: failed to create HDR image", __FUNCTION__);
        vmaDestroyBuffer(renderData.rdAllocator, stagingBuffer, stagingAlloc);
        return false;
    }

    VkCommandBuffer cmd;
    if (!CommandBuffer::init(renderData, cmd))
        return false;

    VkCommandBufferBeginInfo beginInfo{};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

    vkBeginCommandBuffer(cmd, &beginInfo);

    VkImageMemoryBarrier barrier{};
    barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
    barrier.image = texture.image;
    barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    barrier.subresourceRange.baseMipLevel = 0;
    barrier.subresourceRange.levelCount = 1;
    barrier.subresourceRange.baseArrayLayer = 0;
    barrier.subresourceRange.layerCount = 1;

    barrier.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    barrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
    barrier.srcAccessMask = 0;
    barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;

    vkCmdPipelineBarrier(
        cmd,
        VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
        VK_PIPELINE_STAGE_TRANSFER_BIT,
        0, 0, nullptr, 0, nullptr, 1, &barrier
    );

    VkBufferImageCopy copy{};
    copy.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    copy.imageSubresource.mipLevel = 0;
    copy.imageSubresource.baseArrayLayer = 0;
    copy.imageSubresource.layerCount = 1;
    copy.imageExtent = { static_cast<uint32_t>(width), static_cast<uint32_t>(height), 1 };

    vkCmdCopyBufferToImage(cmd, stagingBuffer, texture.image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &copy);

    barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
    barrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
    barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

    vkCmdPipelineBarrier(
        cmd,
        VK_PIPELINE_STAGE_TRANSFER_BIT,
        VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
        0, 0, nullptr, 0, nullptr, 1, &barrier
    );

    vkEndCommandBuffer(cmd);

    VkSubmitInfo submitInfo{};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &cmd;

    vkQueueSubmit(renderData.rdGraphicsQueue, 1, &submitInfo, VK_NULL_HANDLE);
    vkQueueWaitIdle(renderData.rdGraphicsQueue);

    vkFreeCommandBuffers(renderData.rdVkbDevice.device, renderData.rdCommandPool, 1, &cmd);
    vmaDestroyBuffer(renderData.rdAllocator, stagingBuffer, stagingAlloc);

    VkImageViewCreateInfo viewInfo{};
    viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    viewInfo.image = texture.image;
    viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
    viewInfo.format = VK_FORMAT_R16G16B16A16_SFLOAT;
    viewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    viewInfo.subresourceRange.levelCount = 1;
    viewInfo.subresourceRange.layerCount = 1;

    if (vkCreateImageView(renderData.rdVkbDevice.device, &viewInfo, nullptr, &texture.imageView) != VK_SUCCESS)
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
    samplerInfo.minLod = 0.0f;
    samplerInfo.maxLod = 0.0f;

    if (vkCreateSampler(renderData.rdVkbDevice.device, &samplerInfo, nullptr, &texture.sampler) != VK_SUCCESS)
    {
        Logger::log(1, "%s error: failed to create HDR sampler", __FUNCTION__);
        return false;
    }

    Logger::log(1, "%s: loaded HDR texture %s", __FUNCTION__, path.c_str());
    return true;
}

bool Core::Renderer::Cubemap::convertHDRToCubemap(VkRenderData& renderData, VkHDRTextureData& hdrTexture,
    VkCubemapData& cubemapData)
{
    uint32_t cubemapSize = 512;

    VkImageCreateInfo imageInfo{};
    imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    imageInfo.imageType = VK_IMAGE_TYPE_2D;
    imageInfo.format = VK_FORMAT_R16G16B16A16_SFLOAT;
    imageInfo.extent = { cubemapSize, cubemapSize, 1 };
    imageInfo.mipLevels = 1;
    imageInfo.arrayLayers = 6;
    imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
    imageInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
    imageInfo.usage =
        VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT |
        VK_IMAGE_USAGE_TRANSFER_DST_BIT |
        VK_IMAGE_USAGE_SAMPLED_BIT;
    imageInfo.flags = VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT;
    imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;

    VmaAllocationCreateInfo allocInfo{};
    allocInfo.usage = VMA_MEMORY_USAGE_GPU_ONLY;

    if (vmaCreateImage(renderData.rdAllocator, &imageInfo, &allocInfo,
        &cubemapData.image, &cubemapData.imageAlloc, nullptr) != VK_SUCCESS)
    {
        Logger::log(1, "%s error: failed to create cubemap image", __FUNCTION__);
        return false;
    }

    VkImageViewCreateInfo viewInfo{};
    viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    viewInfo.image = cubemapData.image;
    viewInfo.viewType = VK_IMAGE_VIEW_TYPE_CUBE;
    viewInfo.format = imageInfo.format;
    viewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    viewInfo.subresourceRange.levelCount = 1;
    viewInfo.subresourceRange.layerCount = 6;

    vkCreateImageView(renderData.rdVkbDevice.device, &viewInfo, nullptr, &cubemapData.imageView);

    VkSamplerCreateInfo samplerInfo{};
    samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
    samplerInfo.magFilter = VK_FILTER_LINEAR;
    samplerInfo.minFilter = VK_FILTER_LINEAR;
    samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
    samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
    samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
    samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
    samplerInfo.minLod = 0.f;
    samplerInfo.maxLod = 1.f;
    samplerInfo.mipLodBias = 0.f;
    samplerInfo.maxAnisotropy = 1.f;
    samplerInfo.anisotropyEnable = VK_FALSE;
    samplerInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;

    if (vkCreateSampler(renderData.rdVkbDevice.device, &samplerInfo, nullptr, &cubemapData.sampler) != VK_SUCCESS)
    {
        Logger::log(1, "%s error: failed to create cubemap sampler", __FUNCTION__);
        return false;
    }

    VkDescriptorSetLayoutBinding binding{};
    binding.binding = 0;
    binding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    binding.descriptorCount = 1;
    binding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

    VkDescriptorSetLayoutCreateInfo layoutInfo{};
    layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    layoutInfo.bindingCount = 1;
    layoutInfo.pBindings = &binding;

    if (vkCreateDescriptorSetLayout(renderData.rdVkbDevice.device, &layoutInfo, nullptr, &cubemapData.descriptorSetLayout) != VK_SUCCESS)
    {
        Logger::log(1, "%s error: failed to create cubemap descriptor set layout", __FUNCTION__);
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

    if (vkCreateDescriptorPool(renderData.rdVkbDevice.device, &poolInfo, nullptr, &cubemapData.descriptorPool) != VK_SUCCESS)
    {
        Logger::log(1, "%s error: failed to create cubemap descriptor pool", __FUNCTION__);
        return false;
    }

    VkDescriptorSetAllocateInfo allocInfoDS{};
    allocInfoDS.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    allocInfoDS.descriptorPool = cubemapData.descriptorPool;
    allocInfoDS.descriptorSetCount = 1;
    allocInfoDS.pSetLayouts = &cubemapData.descriptorSetLayout;

    if (vkAllocateDescriptorSets(renderData.rdVkbDevice.device, &allocInfoDS, &cubemapData.descriptorSet) != VK_SUCCESS)
    {
        Logger::log(1, "%s error: failed to allocate cubemap descriptor set", __FUNCTION__);
        return false;
    }

    VkDescriptorImageInfo imageInfoDS{};
    imageInfoDS.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    imageInfoDS.imageView = cubemapData.imageView;
    imageInfoDS.sampler = cubemapData.sampler;

    VkWriteDescriptorSet write{};
    write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    write.dstSet = cubemapData.descriptorSet;
    write.dstBinding = 0;
    write.descriptorCount = 1;
    write.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    write.pImageInfo = &imageInfoDS;

    vkUpdateDescriptorSets(renderData.rdVkbDevice.device, 1, &write, 0, nullptr);

    VkImage offscreenImage;
    VmaAllocation offscreenAlloc;

    imageInfo.arrayLayers = 1;
    imageInfo.flags = 0;
    imageInfo.usage =
        VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT |
        VK_IMAGE_USAGE_TRANSFER_SRC_BIT;

    vmaCreateImage(renderData.rdAllocator, &imageInfo, &allocInfo, &offscreenImage, &offscreenAlloc, nullptr);

    VkImageView offscreenView;
    VkImageViewCreateInfo offscreenViewInfo{VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO};
    offscreenViewInfo.image = offscreenImage;
    offscreenViewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
    offscreenViewInfo.format = imageInfo.format;
    offscreenViewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    offscreenViewInfo.subresourceRange.levelCount = 1;
    offscreenViewInfo.subresourceRange.layerCount = 1;

    vkCreateImageView(renderData.rdVkbDevice.device, &viewInfo, nullptr, &offscreenView);

    VkFramebuffer offscreenFramebuffer;
    VkFramebufferCreateInfo offscreenFramebufferInfo{};
    offscreenFramebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
    offscreenFramebufferInfo.renderPass = renderData.rdHDRToCubemapRenderpass;
    offscreenFramebufferInfo.attachmentCount = 1;
    offscreenFramebufferInfo.pAttachments = &offscreenView;
    offscreenFramebufferInfo.width  = cubemapSize;
    offscreenFramebufferInfo.height = cubemapSize;
    offscreenFramebufferInfo.layers = 1;

    if (vkCreateFramebuffer(renderData.rdVkbDevice.device, &offscreenFramebufferInfo, nullptr, &offscreenFramebuffer) != VK_SUCCESS)
    {
        Logger::log(1, "%s error: failed to create cubemap framebuffer", __FUNCTION__);
        return false;
    }

    VkCommandBuffer cmd;
    CommandBuffer::init(renderData, cmd);

    VkCommandBufferBeginInfo begin{};
    begin.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    begin.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

    vkBeginCommandBuffer(cmd, &begin);

    VkImageMemoryBarrier barrier{};
    barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
    barrier.image = cubemapData.image;
    barrier.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    barrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
    barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    barrier.srcAccessMask = 0;
    barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
    barrier.subresourceRange.levelCount = 1;
    barrier.subresourceRange.layerCount = 6;

    vkCmdPipelineBarrier(
        cmd,
        VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
        VK_PIPELINE_STAGE_TRANSFER_BIT,
        0, 0, nullptr, 0, nullptr, 1, &barrier);

    std::vector<VkDescriptorSet> descriptorSets = {
        renderData.rdCaptureUBO.rdUBODescriptorSet,
        hdrTexture.descriptorSet
    };

    for (uint32_t face = 0; face < 6; ++face)
    {
        struct PushConstants
        {
            int faceIndex;
        };
        PushConstants pushConstants{};
        pushConstants.faceIndex = face;

        vkCmdPushConstants(cmd, renderData.rdHDRToCubemapPipelineLayout, VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(PushConstants), &pushConstants);

        VkImageMemoryBarrier offscreenBarrier{VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER};
        offscreenBarrier.image = offscreenImage;
        offscreenBarrier.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        offscreenBarrier.newLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
        offscreenBarrier.srcAccessMask = 0;
        offscreenBarrier.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
        offscreenBarrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        offscreenBarrier.subresourceRange.levelCount = 1;
        offscreenBarrier.subresourceRange.layerCount = 1;

        vkCmdPipelineBarrier(cmd, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
            0, 0, nullptr, 0, nullptr, 1, &offscreenBarrier);

        VkClearValue clear{};
        clear.color = {{0.f, 0.f, 0.f, 1.f}};

        VkRenderPassBeginInfo renderPassBeginInfo{};
        renderPassBeginInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
        renderPassBeginInfo.renderPass = renderData.rdHDRToCubemapRenderpass;
        renderPassBeginInfo.framebuffer = offscreenFramebuffer;
        renderPassBeginInfo.renderArea.extent = { cubemapSize, cubemapSize };
        renderPassBeginInfo.clearValueCount = 1;
        renderPassBeginInfo.pClearValues = &clear;

        vkCmdBeginRenderPass(cmd, &renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);

        VkViewport viewport{0, 0, static_cast<float>(cubemapSize), static_cast<float>(cubemapSize), 0.f, 1.f};
        VkRect2D scissor{{0,0},{cubemapSize,cubemapSize}};

        vkCmdSetViewport(cmd, 0, 1, &viewport);
        vkCmdSetScissor(cmd, 0, 1, &scissor);

        vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, renderData.rdHDRToCubemapPipeline);

        vkCmdBindDescriptorSets(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, renderData.rdHDRToCubemapPipelineLayout,
            0, static_cast<uint32_t>(descriptorSets.size()), descriptorSets.data(),
            0, nullptr);

        VkDeviceSize offset = 0;
        vkCmdBindVertexBuffers(cmd, 0, 1, &renderData.rdVertexBufferData.rdVertexBuffer, &offset);

        vkCmdDraw(cmd, 36, 1, 0, 0);

        vkCmdEndRenderPass(cmd);

        VkImageCopy copy{};
        copy.srcSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        copy.srcSubresource.layerCount = 1;
        copy.dstSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        copy.dstSubresource.baseArrayLayer = face;
        copy.dstSubresource.layerCount = 1;
        copy.extent = { cubemapSize, cubemapSize, 1 };

        VkImageMemoryBarrier barrier{};
        barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
        barrier.oldLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
        barrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
        barrier.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
        barrier.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
        barrier.image = offscreenImage;
        barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        barrier.subresourceRange.levelCount = 1;
        barrier.subresourceRange.layerCount = 1;

        vkCmdPipelineBarrier(
            cmd,
            VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
            VK_PIPELINE_STAGE_TRANSFER_BIT,
            0, 0, nullptr, 0, nullptr, 1, &barrier
        );

        vkCmdCopyImage(cmd, offscreenImage, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
            cubemapData.image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &copy);
    }

    barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
    barrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
    barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

    vkCmdPipelineBarrier(cmd, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
        0, 0, nullptr, 0, nullptr, 1, &barrier);

    vkEndCommandBuffer(cmd);

    VkSubmitInfo submit{};
    submit.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submit.commandBufferCount = 1;
    submit.pCommandBuffers = &cmd;

    vkQueueSubmit(renderData.rdGraphicsQueue, 1, &submit, VK_NULL_HANDLE);
    vkQueueWaitIdle(renderData.rdGraphicsQueue);

    vkDestroyImageView(renderData.rdVkbDevice.device, offscreenView, nullptr);
    vmaDestroyImage(renderData.rdAllocator, offscreenImage, offscreenAlloc);
    vkFreeCommandBuffers(renderData.rdVkbDevice.device, renderData.rdCommandPool, 1, &cmd);
    vkDestroyFramebuffer(renderData.rdVkbDevice.device, offscreenFramebuffer, nullptr);

    Logger::log(1, "%s: HDR converted to cubemap", __FUNCTION__);

    return true;
}

void Core::Renderer::Cubemap::cleanup(VkRenderData& renderData,VkCubemapData& cubemapData)
{
    vkDestroySampler(renderData.rdVkbDevice.device, cubemapData.sampler, nullptr);
    vkDestroyImageView(renderData.rdVkbDevice.device, cubemapData.imageView, nullptr);
    vmaDestroyImage(renderData.rdAllocator, cubemapData.image, cubemapData.imageAlloc);
    vkDestroyDescriptorSetLayout(renderData.rdVkbDevice.device, cubemapData.descriptorSetLayout, nullptr);
    vkDestroyDescriptorPool(renderData.rdVkbDevice.device, cubemapData.descriptorPool, nullptr);
}
