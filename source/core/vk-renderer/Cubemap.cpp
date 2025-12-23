#include "Cubemap.h"
#include "core/tools/Logger.h"
#include "stb_image.h"
#include "core/vk-renderer/buffers/CommandBuffer.h"

bool Core::Renderer::Cubemap::loadHDRTexture(VkRenderData& renderData, VkHDRTextureData& texture, const std::string& path)
{
    stbi_set_flip_vertically_on_load(true);

    int width, height, channels;
    float* hdrData = stbi_loadf(path.c_str(), &width, &height, &channels, STBI_rgb_alpha);

    if (!hdrData)
    {
        Logger::log(1, "%s error: failed to load HDR %s", __FUNCTION__, path.c_str());
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
    barrier.srcAccessMask = 0;
    barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
    barrier.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    barrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
    barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    barrier.subresourceRange.baseMipLevel = 0;
    barrier.subresourceRange.levelCount = 1;
    barrier.subresourceRange.baseArrayLayer = 0;
    barrier.subresourceRange.layerCount = 1;

    vkCmdPipelineBarrier(cmd, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT,
        0, 0, nullptr, 0, nullptr,
        1, &barrier);

    VkBufferImageCopy copy{};
    copy.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    copy.imageSubresource.layerCount = 1;
    copy.imageExtent = { static_cast<uint32_t>(width), static_cast<uint32_t>(height), 1 };

    vkCmdCopyBufferToImage(cmd, stagingBuffer, texture.image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &copy);

    barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
    barrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
    barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

    vkCmdPipelineBarrier(cmd, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
        0, 0, nullptr, 0, nullptr,
        1, &barrier);

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
    viewInfo.format = VK_FORMAT_R32G32B32A32_SFLOAT;
    viewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    viewInfo.subresourceRange.baseMipLevel = 0;
    viewInfo.subresourceRange.levelCount = 1;
    viewInfo.subresourceRange.baseArrayLayer = 0;
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
    samplerInfo.minLod = 0.f;
    samplerInfo.maxLod = 1.f;

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
    constexpr uint32_t cubemapSize = 512;

    VkImageCreateInfo cubemapImageInfo{};
    cubemapImageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    cubemapImageInfo.imageType = VK_IMAGE_TYPE_2D;
    cubemapImageInfo.format = VK_FORMAT_R32G32B32A32_SFLOAT;
    cubemapImageInfo.extent = { cubemapSize, cubemapSize, 1 };
    cubemapImageInfo.mipLevels = 1;
    cubemapImageInfo.arrayLayers = 6;
    cubemapImageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
    cubemapImageInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
    cubemapImageInfo.usage =
        VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT |
        VK_IMAGE_USAGE_TRANSFER_DST_BIT |
        VK_IMAGE_USAGE_SAMPLED_BIT;
    cubemapImageInfo.flags = VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT;
    cubemapImageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;

    VmaAllocationCreateInfo allocInfo{};
    allocInfo.usage = VMA_MEMORY_USAGE_GPU_ONLY;

    if (vmaCreateImage(renderData.rdAllocator, &cubemapImageInfo, &allocInfo,
        &cubemapData.image, &cubemapData.imageAlloc, nullptr) != VK_SUCCESS)
    {
        Logger::log(1, "%s error: failed to create cubemap image", __FUNCTION__);
        return false;
    }

    VkImageViewCreateInfo cubemapViewInfo{};
    cubemapViewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    cubemapViewInfo.image = cubemapData.image;
    cubemapViewInfo.viewType = VK_IMAGE_VIEW_TYPE_CUBE;
    cubemapViewInfo.format = cubemapImageInfo.format;
    cubemapViewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    cubemapViewInfo.subresourceRange.levelCount = 1;
    cubemapViewInfo.subresourceRange.baseArrayLayer = 0;
    cubemapViewInfo.subresourceRange.layerCount = 6;

    vkCreateImageView(renderData.rdVkbDevice.device, &cubemapViewInfo, nullptr, &cubemapData.imageView);

    VkImageCreateInfo offscreenImageInfo = cubemapImageInfo;
    offscreenImageInfo.arrayLayers = 1;
    offscreenImageInfo.flags = 0;
    offscreenImageInfo.usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT;

    VkImage offscreenImage;
    VmaAllocation offscreenAlloc;
    vmaCreateImage(renderData.rdAllocator, &offscreenImageInfo, &allocInfo, &offscreenImage, &offscreenAlloc, nullptr);
    
    VkImageViewCreateInfo offscreenViewInfo{};
    offscreenViewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    offscreenViewInfo.image = offscreenImage;
    offscreenViewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
    offscreenViewInfo.format = offscreenImageInfo.format;
    offscreenViewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    offscreenViewInfo.subresourceRange.baseMipLevel = 0;
    offscreenViewInfo.subresourceRange.levelCount = 1;
    offscreenViewInfo.subresourceRange.baseArrayLayer = 0;
    offscreenViewInfo.subresourceRange.layerCount = 1;

    VkImageView offscreenImageView;
    vkCreateImageView(renderData.rdVkbDevice.device, &offscreenViewInfo, nullptr, &offscreenImageView);

    VkFramebufferCreateInfo framebufferCreateInfo{};
    framebufferCreateInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
    framebufferCreateInfo.renderPass = renderData.rdHDRToCubemapRenderpass;
    framebufferCreateInfo.attachmentCount = 1;
    framebufferCreateInfo.pAttachments = &offscreenImageView;
    framebufferCreateInfo.width = cubemapSize;
    framebufferCreateInfo.height = cubemapSize;
    framebufferCreateInfo.layers = 1;

    VkFramebuffer framebuffer;
    vkCreateFramebuffer(renderData.rdVkbDevice.device, &framebufferCreateInfo, nullptr, &framebuffer);

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
    barrier.srcAccessMask = 0;
    barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
    barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    barrier.subresourceRange.baseMipLevel = 0;
    barrier.subresourceRange.levelCount = 1;
    barrier.subresourceRange.baseArrayLayer  = 0;
    barrier.subresourceRange.layerCount = 6;

    vkCmdPipelineBarrier(cmd, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, 
        VK_PIPELINE_STAGE_TRANSFER_BIT | VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, 0, 
        0, nullptr, 0, nullptr, 1,
        &barrier);
    
    VkImageMemoryBarrier offscreenToColor{};
    offscreenToColor.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
    offscreenToColor.image = offscreenImage;
    offscreenToColor.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    offscreenToColor.newLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    offscreenToColor.srcAccessMask = 0;
    offscreenToColor.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
    offscreenToColor.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    offscreenToColor.subresourceRange.baseMipLevel = 0;
    offscreenToColor.subresourceRange.levelCount = 1;
    offscreenToColor.subresourceRange.baseArrayLayer = 0;
    offscreenToColor.subresourceRange.layerCount = 1;

    vkCmdPipelineBarrier(cmd, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
        0, 0, nullptr, 0, nullptr,
        1, &offscreenToColor);

    for (uint32_t face = 0; face < 6; ++face)
    {
        vkCmdPushConstants(cmd, renderData.rdHDRToCubemapPipelineLayout, VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(int), &face);

        VkClearValue clear{};
        clear.color = {{0,0,0,1}};

        VkRenderPassBeginInfo renderPassBeginInfo{};
        renderPassBeginInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
        renderPassBeginInfo.renderPass = renderData.rdHDRToCubemapRenderpass;
        renderPassBeginInfo.framebuffer = framebuffer;
        renderPassBeginInfo.renderArea.offset = {0,0};
        renderPassBeginInfo.renderArea.extent = { cubemapSize, cubemapSize };
        renderPassBeginInfo.clearValueCount = 1;
        renderPassBeginInfo.pClearValues = &clear;

        vkCmdBeginRenderPass(cmd, &renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);

        VkViewport viewport{};
        viewport.x = 0.0f;
        viewport.y = 0.0f;
        viewport.width = static_cast<float>(cubemapSize);
        viewport.height =  static_cast<float>(cubemapSize);
        viewport.minDepth = 0.f;
        viewport.maxDepth = 1.f;

        VkRect2D scissor{};
        scissor.offset = {0, 0};
        scissor.extent = {cubemapSize, cubemapSize};

        vkCmdSetViewport(cmd, 0, 1, &viewport);
        vkCmdSetScissor(cmd, 0, 1, &scissor);
        
        vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, renderData.rdHDRToCubemapPipeline);

        std::vector descriptorSets = {renderData.rdCaptureUBO.rdUBODescriptorSet, hdrTexture.descriptorSet};

        vkCmdBindDescriptorSets(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, renderData.rdHDRToCubemapPipelineLayout,
           0, static_cast<uint32_t>(descriptorSets.size()), descriptorSets.data(),
           0, nullptr);

        vkCmdDraw(cmd, 36, 1, 0, 0);
        vkCmdEndRenderPass(cmd);

        VkImageMemoryBarrier toSrcImageMemoryBarrier{};
        toSrcImageMemoryBarrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
        toSrcImageMemoryBarrier.image = offscreenImage;
        toSrcImageMemoryBarrier.oldLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
        toSrcImageMemoryBarrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
        toSrcImageMemoryBarrier.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
        toSrcImageMemoryBarrier.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
        toSrcImageMemoryBarrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        toSrcImageMemoryBarrier.subresourceRange.baseMipLevel = 0;
        toSrcImageMemoryBarrier.subresourceRange.levelCount = 1;
        toSrcImageMemoryBarrier.subresourceRange.baseArrayLayer = 0;
        toSrcImageMemoryBarrier.subresourceRange.layerCount = 1;

        vkCmdPipelineBarrier(cmd, VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT,
            0, 0, nullptr, 0, nullptr,
            1, &toSrcImageMemoryBarrier);

        VkImageCopy copy{};
        copy.srcSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        copy.srcSubresource.layerCount = 1;
        copy.dstSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        copy.dstSubresource.baseArrayLayer = face;
        copy.dstSubresource.layerCount = 1;
        copy.extent = { cubemapSize, cubemapSize, 1 };

        vkCmdCopyImage(cmd, offscreenImage, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
            cubemapData.image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &copy);
    }

    VkImageMemoryBarrier cubeToSampleImageMemoryBarrier{};
    cubeToSampleImageMemoryBarrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
    cubeToSampleImageMemoryBarrier.image = cubemapData.image;
    cubeToSampleImageMemoryBarrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
    cubeToSampleImageMemoryBarrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    cubeToSampleImageMemoryBarrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
    cubeToSampleImageMemoryBarrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
    cubeToSampleImageMemoryBarrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    cubeToSampleImageMemoryBarrier.subresourceRange.levelCount = 1;
    cubeToSampleImageMemoryBarrier.subresourceRange.layerCount = 6;

    vkCmdPipelineBarrier(cmd, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
        0, 0, nullptr, 0, nullptr, 
        1, &cubeToSampleImageMemoryBarrier);

    vkEndCommandBuffer(cmd);

    VkSubmitInfo submit{};
    submit.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submit.commandBufferCount = 1;
    submit.pCommandBuffers = &cmd;

    vkQueueSubmit(renderData.rdGraphicsQueue, 1, &submit, VK_NULL_HANDLE);
    vkQueueWaitIdle(renderData.rdGraphicsQueue);

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

    vkCreateDescriptorSetLayout(renderData.rdVkbDevice.device, &layoutInfo, nullptr, &cubemapData.descriptorSetLayout);

    VkDescriptorPoolSize poolSize{};
    poolSize.type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    poolSize.descriptorCount = 1;

    VkDescriptorPoolCreateInfo poolInfo{};
    poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    poolInfo.poolSizeCount = 1;
    poolInfo.pPoolSizes = &poolSize;
    poolInfo.maxSets = 1;

    vkCreateDescriptorPool(renderData.rdVkbDevice.device, &poolInfo, nullptr, &cubemapData.descriptorPool);

    VkDescriptorSetAllocateInfo allocInfoDS{};
    allocInfoDS.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    allocInfoDS.descriptorPool = cubemapData.descriptorPool;
    allocInfoDS.descriptorSetCount = 1;
    allocInfoDS.pSetLayouts = &cubemapData.descriptorSetLayout;

    vkAllocateDescriptorSets(renderData.rdVkbDevice.device, &allocInfoDS, &cubemapData.descriptorSet);

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
    
    Logger::log(1, "%s: HDR converted to cubemap", __FUNCTION__);

    return true;
}

bool Core::Renderer::Cubemap::convertCubemapToIrradiance(VkRenderData& renderData, VkCubemapData& cubemapData,
    VkCubemapData& irradianceData)
{
     constexpr uint32_t irradianceSize = 32;

    VkImageCreateInfo imageInfo{};
    imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    imageInfo.imageType = VK_IMAGE_TYPE_2D;
    imageInfo.format = VK_FORMAT_R32G32B32A32_SFLOAT;
    imageInfo.extent = { irradianceSize, irradianceSize, 1 };
    imageInfo.mipLevels = 1;
    imageInfo.arrayLayers = 6;
    imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
    imageInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
    imageInfo.usage =
        VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT |
        VK_IMAGE_USAGE_TRANSFER_DST_BIT |
        VK_IMAGE_USAGE_SAMPLED_BIT;
    imageInfo.flags = VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT;

    VmaAllocationCreateInfo allocInfo{};
    allocInfo.usage = VMA_MEMORY_USAGE_GPU_ONLY;

    vmaCreateImage(renderData.rdAllocator, &imageInfo, &allocInfo,
                   &irradianceData.image, &irradianceData.imageAlloc, nullptr);

    VkImageViewCreateInfo viewInfo{};
    viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    viewInfo.image = irradianceData.image;
    viewInfo.viewType = VK_IMAGE_VIEW_TYPE_CUBE;
    viewInfo.format = imageInfo.format;
    viewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    viewInfo.subresourceRange.levelCount = 1;
    viewInfo.subresourceRange.layerCount = 6;

    vkCreateImageView(renderData.rdVkbDevice.device, &viewInfo, nullptr,
                      &irradianceData.imageView);

    VkImage offscreenImage;
    VmaAllocation offscreenAlloc;

    imageInfo.arrayLayers = 1;
    imageInfo.flags = 0;
    imageInfo.usage =
        VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT |
        VK_IMAGE_USAGE_TRANSFER_SRC_BIT;

    vmaCreateImage(renderData.rdAllocator, &imageInfo, &allocInfo, &offscreenImage, &offscreenAlloc, nullptr);

    VkImageView offscreenView;
    VkImageViewCreateInfo offscreenViewInfo{};
    offscreenViewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    offscreenViewInfo.image = offscreenImage;
    offscreenViewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
    offscreenViewInfo.format = imageInfo.format;
    offscreenViewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    offscreenViewInfo.subresourceRange.levelCount = 1;
    offscreenViewInfo.subresourceRange.layerCount = 1;

    vkCreateImageView(renderData.rdVkbDevice.device, &offscreenViewInfo, nullptr, &offscreenView);

    VkFramebuffer framebuffer;
    VkFramebufferCreateInfo fb{};
    fb.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
    fb.renderPass = renderData.rdIrradianceRenderpass;
    fb.attachmentCount = 1;
    fb.pAttachments = &offscreenView;
    fb.width = irradianceSize;
    fb.height = irradianceSize;
    fb.layers = 1;

    vkCreateFramebuffer(renderData.rdVkbDevice.device, &fb, nullptr, &framebuffer);

    VkCommandBuffer cmd;
    CommandBuffer::init(renderData, cmd);

    VkCommandBufferBeginInfo begin{};
    begin.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    begin.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
    vkBeginCommandBuffer(cmd, &begin);

    VkImageMemoryBarrier barrier{};
    barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
    barrier.image = irradianceData.image;
    barrier.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    barrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
    barrier.srcAccessMask = 0;
    barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
    barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    barrier.subresourceRange.levelCount = 1;
    barrier.subresourceRange.layerCount = 6;

    vkCmdPipelineBarrier(cmd,
        VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
        VK_PIPELINE_STAGE_TRANSFER_BIT,
        0, 0, nullptr, 0, nullptr, 1, &barrier);

    for (uint32_t face = 0; face < 6; ++face)
    {
        vkCmdPushConstants(
            cmd,
            renderData.rdIrradiancePipelineLayout,
            VK_SHADER_STAGE_VERTEX_BIT,
            0, sizeof(uint32_t),
            &face
        );

        VkRenderPassBeginInfo rp{};
        rp.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
        rp.renderPass = renderData.rdIrradianceRenderpass;
        rp.framebuffer = framebuffer;
        rp.renderArea.extent = { irradianceSize, irradianceSize };

        VkClearValue clear{};
        clear.color = {{0,0,0,1}};
        rp.clearValueCount = 1;
        rp.pClearValues = &clear;

        vkCmdBeginRenderPass(cmd, &rp, VK_SUBPASS_CONTENTS_INLINE);
        vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, renderData.rdIrradiancePipeline);

        VkDescriptorSet sets[] = {
            renderData.rdCaptureUBO.rdUBODescriptorSet,
            cubemapData.descriptorSet
        };

        vkCmdBindDescriptorSets(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, renderData.rdIrradiancePipelineLayout,
            0, 2, sets, 0, nullptr);

        vkCmdDraw(cmd, 36, 1, 0, 0);
        vkCmdEndRenderPass(cmd);

        VkImageCopy copy{};
        copy.srcSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        copy.dstSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        copy.dstSubresource.baseArrayLayer = face;
        copy.extent = { irradianceSize, irradianceSize, 1 };

        vkCmdCopyImage(
            cmd,
            offscreenImage, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
            irradianceData.image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
            1, &copy
        );
    }

    vkEndCommandBuffer(cmd);

    VkSubmitInfo submit{};
    submit.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submit.commandBufferCount = 1;
    submit.pCommandBuffers = &cmd;

    vkQueueSubmit(renderData.rdGraphicsQueue, 1, &submit, VK_NULL_HANDLE);
    vkQueueWaitIdle(renderData.rdGraphicsQueue);

    Logger::log(1, "%s: Irradiance cubemap generated", __FUNCTION__);

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
