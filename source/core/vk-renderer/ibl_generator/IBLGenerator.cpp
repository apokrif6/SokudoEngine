#include "IBLGenerator.h"

#include "core/engine/Engine.h"
#include "core/tools/Logger.h"
#include "core/vk-renderer/buffers/CommandBuffer.h"
#include "HDRToCubemapRenderpass.h"
#include "core/vk-renderer/Texture.h"
#include "core/vk-renderer/debug/DebugUtils.h"
#include "core/vk-renderer/pipelines/Pipeline.h"

bool Core::Renderer::IBLGenerator::init(VkRenderData& renderData)
{
    if (!HDRToCubemapRenderpass::init(renderData, renderData.rdHDRToCubemapRenderpass))
    {
        Logger::log(1, "%s error: could not init HDR to Cubemap renderpass\n", __FUNCTION__);
        return false;
    }

    if (!HDRToCubemapRenderpass::init(renderData, renderData.rdIBLData.rdIBLRenderpass))
    {
        Logger::log(1, "%s error: could not init IBL renderpass\n", __FUNCTION__);
        return false;
    }

    if (!createDescriptorForHDR(renderData))
    {
        return false;
    }

    if (!createStaticCubemapLayout(renderData, renderData.rdIBLData.rdSingleCubemapDescriptorLayout))
    {
        return false;
    }

    if (!createHDRToCubemapPipeline(renderData) ||
        !createIrradiancePipeline(renderData) ||
        !createPrefilterPipeline(renderData) ||
        !createBRDFLUTPipeline(renderData))
    {
        return false;
    }

    return true;
}

bool Core::Renderer::IBLGenerator::generateIBL(VkRenderData& renderData)
{
    if (!convertHDRToCubemap(renderData, renderData.rdHDRTexture, renderData.rdSkyboxData))
    {
        Logger::log(1, "IBL Error: Failed to convert HDR to Cubemap");
        return false;
    }

    if (!convertCubemapToIrradiance(renderData, renderData.rdSkyboxData, renderData.rdIBLData.rdIrradianceMap))
    {
        Logger::log(1, "IBL Error: Failed to generate Irradiance map");
        return false;
    }

    if (!convertCubemapToPrefilteredMap(renderData, renderData.rdSkyboxData, renderData.rdIBLData.rdPrefilterMap))
    {
        Logger::log(1, "IBL Error: Failed to generate Prefiltered map");
        return false;
    }

    if (!generateBRDFLUT(renderData, renderData.rdIBLData.rdBRDFLUT))
    {
        Logger::log(1, "IBL Error: Failed to generate BRDF LUT");
        return false;
    }

    Logger::log(1, "IBL Generation completed successfully");

    return true;
}

bool Core::Renderer::IBLGenerator::createDescriptorForHDR(VkRenderData& renderData)
{
    VkDescriptorSetLayoutBinding samplerBinding{};
    samplerBinding.binding = 0;
    samplerBinding.descriptorCount = 1;
    samplerBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    samplerBinding.pImmutableSamplers = nullptr;
    samplerBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

    VkDescriptorSetLayoutCreateInfo layoutInfo{};
    layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    layoutInfo.bindingCount = 1;
    layoutInfo.pBindings = &samplerBinding;

    if (vkCreateDescriptorSetLayout(renderData.rdVkbDevice.device, &layoutInfo, nullptr,
                                &renderData.rdHDRTexture.descriptorSetLayout) != VK_SUCCESS)
    {
        Logger::log(1, "%s error: failed to create descriptor set layout\n", __FUNCTION__);
        return false;
    }

    constexpr std::string_view descriptorSetLayoutObjectName = "Descriptor Set Layout IBL HDR";
    vmaSetAllocationName(renderData.rdAllocator, renderData.rdHDRTexture.imageAlloc, descriptorSetLayoutObjectName.data());
    Debug::setObjectName(renderData.rdVkbDevice.device, reinterpret_cast<uint64_t>(renderData.rdHDRTexture.descriptorSetLayout),
               VK_OBJECT_TYPE_DESCRIPTOR_SET_LAYOUT, descriptorSetLayoutObjectName.data());

    VkDescriptorPoolSize poolSize{};
    poolSize.type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    poolSize.descriptorCount = 1;

    VkDescriptorPoolCreateInfo poolInfo{};
    poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    poolInfo.poolSizeCount = 1;
    poolInfo.pPoolSizes = &poolSize;
    poolInfo.maxSets = 1;

    if (vkCreateDescriptorPool(renderData.rdVkbDevice.device, &poolInfo, nullptr,
                           &renderData.rdHDRTexture.descriptorPool) != VK_SUCCESS)
    {
        Logger::log(1, "%s error: failed to create descriptor pool", __FUNCTION__);
        return false;
    }

    constexpr std::string_view descriptorPoolObjectName = "Descriptor Pool IBL HDR texture";
    vmaSetAllocationName(renderData.rdAllocator, renderData.rdHDRTexture.imageAlloc, descriptorPoolObjectName.data());
    Debug::setObjectName(renderData.rdVkbDevice.device, reinterpret_cast<uint64_t>(renderData.rdHDRTexture.descriptorPool),
                         VK_OBJECT_TYPE_DESCRIPTOR_POOL, descriptorPoolObjectName.data());

    VkDescriptorSetAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    allocInfo.descriptorPool = renderData.rdHDRTexture.descriptorPool;
    allocInfo.descriptorSetCount = 1;
    allocInfo.pSetLayouts = &renderData.rdHDRTexture.descriptorSetLayout;

    if (vkAllocateDescriptorSets(renderData.rdVkbDevice.device, &allocInfo,
                             &renderData.rdHDRTexture.descriptorSet) != VK_SUCCESS)
    {
        Logger::log(1, "%s error: failed to allocate descriptor set", __FUNCTION__);
        return false;
    }

    VkDescriptorImageInfo imageInfo{};
    imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    imageInfo.imageView = renderData.rdHDRTexture.imageView;
    imageInfo.sampler = renderData.rdHDRTexture.sampler;

    VkWriteDescriptorSet write{};
    write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    write.dstSet = renderData.rdHDRTexture.descriptorSet;
    write.dstBinding = 0;
    write.descriptorCount = 1;
    write.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    write.pImageInfo = &imageInfo;

    vkUpdateDescriptorSets(renderData.rdVkbDevice.device, 1, &write, 0,
        nullptr);

    return true;
}

bool Core::Renderer::IBLGenerator::createStaticCubemapLayout(VkRenderData& renderData, VkDescriptorSetLayout& layout)
{
    VkDescriptorSetLayoutBinding samplerBinding{};
    samplerBinding.binding = 0;
    samplerBinding.descriptorCount = 1;
    samplerBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    samplerBinding.pImmutableSamplers = nullptr;
    samplerBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

    VkDescriptorSetLayoutCreateInfo layoutInfo{};
    layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    layoutInfo.bindingCount = 1;
    layoutInfo.pBindings = &samplerBinding;

    if (vkCreateDescriptorSetLayout(renderData.rdVkbDevice.device, &layoutInfo, nullptr, &layout) != VK_SUCCESS)
    {
        Logger::log(1, "%s error: could not create cubemap descriptor set layout", __FUNCTION__);
        return false;
    }

    constexpr std::string_view descriptorSetLayoutObjectName = "Descriptor Set Layout IBL Static Cubemap";
    Debug::setObjectName(renderData.rdVkbDevice.device, reinterpret_cast<uint64_t>(layout),
               VK_OBJECT_TYPE_DESCRIPTOR_SET_LAYOUT, descriptorSetLayoutObjectName.data());

    return true;
}

bool Core::Renderer::IBLGenerator::convertHDRToCubemap(VkRenderData& renderData, VkTextureData& texture,
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

    vmaSetAllocationName(renderData.rdAllocator, offscreenAlloc, "Convert HDR Cubemap Offscreen Alloc");

    VkImageViewCreateInfo offscreenImageViewInfo{};
    offscreenImageViewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    offscreenImageViewInfo.image = offscreenImage;
    offscreenImageViewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
    offscreenImageViewInfo.format = offscreenImageInfo.format;
    offscreenImageViewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    offscreenImageViewInfo.subresourceRange.baseMipLevel = 0;
    offscreenImageViewInfo.subresourceRange.levelCount = 1;
    offscreenImageViewInfo.subresourceRange.baseArrayLayer = 0;
    offscreenImageViewInfo.subresourceRange.layerCount = 1;

    VkImageView offscreenImageView;
    vkCreateImageView(renderData.rdVkbDevice.device, &offscreenImageViewInfo, nullptr, &offscreenImageView);

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
    {
        Debug::Marker marker(renderData.rdVkbDevice.device, cmd, "Convert HDR To Cubemap", Debug::Colors::Orange);

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

            std::vector descriptorSets = {renderData.rdCaptureUBO.rdUBODescriptorSet, texture.descriptorSet};

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
    }
    vkEndCommandBuffer(cmd);

    VkSubmitInfo submit{};
    submit.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submit.commandBufferCount = 1;
    submit.pCommandBuffers = &cmd;

    vkQueueSubmit(renderData.rdGraphicsQueue, 1, &submit, VK_NULL_HANDLE);
    vkQueueWaitIdle(renderData.rdGraphicsQueue);

    vkDestroyFramebuffer(renderData.rdVkbDevice.device, framebuffer, nullptr);
    vkDestroyImageView(renderData.rdVkbDevice.device, offscreenImageView, nullptr);
    vmaDestroyImage(renderData.rdAllocator, offscreenImage, offscreenAlloc);

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

    constexpr std::string_view descriptorSetLayoutObjectName = "Descriptor Set Layout IBL Cubemap Data";
    vmaSetAllocationName(renderData.rdAllocator, cubemapData.imageAlloc, descriptorSetLayoutObjectName.data());
    Debug::setObjectName(renderData.rdVkbDevice.device, reinterpret_cast<uint64_t>(cubemapData.descriptorSetLayout),
               VK_OBJECT_TYPE_DESCRIPTOR_SET_LAYOUT, descriptorSetLayoutObjectName.data());

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
        Logger::log(1, "%s error: failed to create descriptor pool", __FUNCTION__);
        return false;
    }

    constexpr std::string_view descriptorPoolObjectName = "Descriptor Pool IBL Cubemap Data";
    vmaSetAllocationName(renderData.rdAllocator, cubemapData.imageAlloc, descriptorPoolObjectName.data());
    Debug::setObjectName(renderData.rdVkbDevice.device, reinterpret_cast<uint64_t>(cubemapData.descriptorPool),
                        VK_OBJECT_TYPE_DESCRIPTOR_POOL, descriptorPoolObjectName.data());

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

bool Core::Renderer::IBLGenerator::convertCubemapToIrradiance(VkRenderData& renderData, VkCubemapData& cubemapData,
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

    vmaSetAllocationName(renderData.rdAllocator, offscreenAlloc, "Convert Cubemap to Irradiance Offscreen Alloc");

    VkImageView offscreenImageView;
    VkImageViewCreateInfo offscreenImageViewInfo{};
    offscreenImageViewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    offscreenImageViewInfo.image = offscreenImage;
    offscreenImageViewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
    offscreenImageViewInfo.format = imageInfo.format;
    offscreenImageViewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    offscreenImageViewInfo.subresourceRange.levelCount = 1;
    offscreenImageViewInfo.subresourceRange.layerCount = 1;

    vkCreateImageView(renderData.rdVkbDevice.device, &offscreenImageViewInfo, nullptr, &offscreenImageView);

    VkFramebuffer framebuffer;
    VkFramebufferCreateInfo fb{};
    fb.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
    fb.renderPass = renderData.rdIBLData.rdIBLRenderpass;
    fb.attachmentCount = 1;
    fb.pAttachments = &offscreenImageView;
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
    {
        Debug::Marker marker(renderData.rdVkbDevice.device, cmd, "Convert Cubemap To Irradiance", Debug::Colors::Cyan);

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
            rp.renderPass = renderData.rdIBLData.rdIBLRenderpass;
            rp.framebuffer = framebuffer;
            rp.renderArea.extent = { irradianceSize, irradianceSize };

            VkClearValue clear{};
            clear.color = {{0,0,0,1}};
            rp.clearValueCount = 1;
            rp.pClearValues = &clear;

            vkCmdBeginRenderPass(cmd, &rp, VK_SUBPASS_CONTENTS_INLINE);

            VkViewport viewport{};
            viewport.x = 0.0f;
            viewport.y = 0.0f;
            viewport.width = static_cast<float>(irradianceSize);
            viewport.height =  static_cast<float>(irradianceSize);
            viewport.minDepth = 0.f;
            viewport.maxDepth = 1.f;

            VkRect2D scissor{};
            scissor.offset = {0, 0};
            scissor.extent = {irradianceSize, irradianceSize};

            vkCmdSetViewport(cmd, 0, 1, &viewport);
            vkCmdSetScissor(cmd, 0, 1, &scissor);

            vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, renderData.rdIrradiancePipeline);

            VkDescriptorSet sets[] = {
                renderData.rdCaptureUBO.rdUBODescriptorSet,
                cubemapData.descriptorSet
            };

            vkCmdBindDescriptorSets(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, renderData.rdIrradiancePipelineLayout,
                0, 2, sets, 0, nullptr);

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
            copy.extent = { irradianceSize, irradianceSize, 1 };

            vkCmdCopyImage(cmd, offscreenImage, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
                irradianceData.image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &copy);
        }

        VkImageMemoryBarrier irradianceToSample{};
        irradianceToSample.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
        irradianceToSample.image = irradianceData.image;
        irradianceToSample.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
        irradianceToSample.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        irradianceToSample.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
        irradianceToSample.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
        irradianceToSample.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        irradianceToSample.subresourceRange.baseMipLevel = 0;
        irradianceToSample.subresourceRange.levelCount = 1;
        irradianceToSample.subresourceRange.baseArrayLayer = 0;
        irradianceToSample.subresourceRange.layerCount = 6;

        vkCmdPipelineBarrier(cmd, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
            0, 0, nullptr, 0, nullptr,
            1, &irradianceToSample);
    }
    vkEndCommandBuffer(cmd);

    VkSubmitInfo submit{};
    submit.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submit.commandBufferCount = 1;
    submit.pCommandBuffers = &cmd;

    vkQueueSubmit(renderData.rdGraphicsQueue, 1, &submit, VK_NULL_HANDLE);
    vkQueueWaitIdle(renderData.rdGraphicsQueue);

    vkDestroyFramebuffer(renderData.rdVkbDevice.device, framebuffer, nullptr);
    vkDestroyImageView(renderData.rdVkbDevice.device, offscreenImageView, nullptr);
    vmaDestroyImage(renderData.rdAllocator, offscreenImage, offscreenAlloc);

    VkSamplerCreateInfo samplerInfo{};
    samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
    samplerInfo.magFilter = VK_FILTER_LINEAR;
    samplerInfo.minFilter = VK_FILTER_LINEAR;
    samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
    samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
    samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
    samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
    samplerInfo.minLod = 0.0f;
    samplerInfo.maxLod = 1.0f;

    if (vkCreateSampler(renderData.rdVkbDevice.device, &samplerInfo, nullptr, &irradianceData.sampler) != VK_SUCCESS)
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

    vkCreateDescriptorSetLayout(renderData.rdVkbDevice.device, &layoutInfo, nullptr, &irradianceData.descriptorSetLayout);

    constexpr std::string_view descriptorSetLayoutObjectName = "Descriptor Set Layout IBL Irradiance";
    vmaSetAllocationName(renderData.rdAllocator, irradianceData.imageAlloc, descriptorSetLayoutObjectName.data());
    Debug::setObjectName(renderData.rdVkbDevice.device, reinterpret_cast<uint64_t>(irradianceData.descriptorSetLayout),
               VK_OBJECT_TYPE_DESCRIPTOR_SET_LAYOUT, descriptorSetLayoutObjectName.data());

    VkDescriptorPoolSize poolSize{};
    poolSize.type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    poolSize.descriptorCount = 1;

    VkDescriptorPoolCreateInfo poolInfo{};
    poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    poolInfo.poolSizeCount = 1;
    poolInfo.pPoolSizes = &poolSize;
    poolInfo.maxSets = 1;

    if (vkCreateDescriptorPool(renderData.rdVkbDevice.device, &poolInfo, nullptr, &irradianceData.descriptorPool) != VK_SUCCESS)
    {
        Logger::log(1, "%s error: failed to create descriptor pool", __FUNCTION__);
        return false;
    }

    constexpr std::string_view descriptorPoolObjectName = "Descriptor Pool IBL Irradiance Data";
    vmaSetAllocationName(renderData.rdAllocator, irradianceData.imageAlloc, descriptorPoolObjectName.data());
    Debug::setObjectName(renderData.rdVkbDevice.device, reinterpret_cast<uint64_t>(irradianceData.descriptorPool),
                       VK_OBJECT_TYPE_DESCRIPTOR_POOL, descriptorPoolObjectName.data());

    VkDescriptorSetAllocateInfo allocInfoDS{};
    allocInfoDS.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    allocInfoDS.descriptorPool = irradianceData.descriptorPool;
    allocInfoDS.descriptorSetCount = 1;
    allocInfoDS.pSetLayouts = &irradianceData.descriptorSetLayout;

    vkAllocateDescriptorSets(renderData.rdVkbDevice.device, &allocInfoDS, &irradianceData.descriptorSet);

    VkDescriptorImageInfo imageInfoDS{};
    imageInfoDS.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    imageInfoDS.imageView = irradianceData.imageView;
    imageInfoDS.sampler = irradianceData.sampler;

    VkWriteDescriptorSet write{};
    write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    write.dstSet = irradianceData.descriptorSet;
    write.dstBinding = 0;
    write.descriptorCount = 1;
    write.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    write.pImageInfo = &imageInfoDS;

    vkUpdateDescriptorSets(renderData.rdVkbDevice.device, 1, &write, 0, nullptr);

    Logger::log(1, "%s: Irradiance cubemap generated", __FUNCTION__);

    return true;
}

bool Core::Renderer::IBLGenerator::convertCubemapToPrefilteredMap(VkRenderData& renderData, VkCubemapData& cubemapData,
    VkCubemapData& prefilteredMapData)
{
    constexpr uint32_t prefilteredMapSize = 512;
    constexpr uint32_t maxMipLevels = 10;

    VkImageCreateInfo imageInfo{};
    imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    imageInfo.imageType = VK_IMAGE_TYPE_2D;
    imageInfo.format = VK_FORMAT_R32G32B32A32_SFLOAT;
    imageInfo.extent = { prefilteredMapSize, prefilteredMapSize, 1 };
    imageInfo.mipLevels = maxMipLevels;
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
                   &prefilteredMapData.image, &prefilteredMapData.imageAlloc, nullptr);

    VkImageViewCreateInfo viewInfo{};
    viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    viewInfo.image = prefilteredMapData.image;
    viewInfo.viewType = VK_IMAGE_VIEW_TYPE_CUBE;
    viewInfo.format = imageInfo.format;
    viewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    viewInfo.subresourceRange.baseMipLevel = 0;
    viewInfo.subresourceRange.levelCount = maxMipLevels;
    viewInfo.subresourceRange.layerCount = 6;

    vkCreateImageView(renderData.rdVkbDevice.device, &viewInfo, nullptr,
                      &prefilteredMapData.imageView);

    VkImage offscreenImage;
    VmaAllocation offscreenAlloc;

    imageInfo.mipLevels = 1;
    imageInfo.arrayLayers = 1;
    imageInfo.flags = 0;
    imageInfo.usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT;

    vmaCreateImage(renderData.rdAllocator, &imageInfo, &allocInfo, &offscreenImage, &offscreenAlloc, nullptr);

    vmaSetAllocationName(renderData.rdAllocator, offscreenAlloc, "Convert Cubemap to Prefiltered Offscreen Alloc");

    VkImageView offscreenImageView;
    VkImageViewCreateInfo offscreenImageViewInfo{};
    offscreenImageViewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    offscreenImageViewInfo.image = offscreenImage;
    offscreenImageViewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
    offscreenImageViewInfo.format = imageInfo.format;
    offscreenImageViewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    offscreenImageViewInfo.subresourceRange.levelCount = 1;
    offscreenImageViewInfo.subresourceRange.layerCount = 1;

    vkCreateImageView(renderData.rdVkbDevice.device, &offscreenImageViewInfo, nullptr, &offscreenImageView);

    VkFramebuffer framebuffer;
    VkFramebufferCreateInfo fb{};
    fb.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
    fb.renderPass = renderData.rdIBLData.rdIBLRenderpass;
    fb.attachmentCount = 1;
    fb.pAttachments = &offscreenImageView;
    fb.width = prefilteredMapSize;
    fb.height = prefilteredMapSize;
    fb.layers = 1;

    vkCreateFramebuffer(renderData.rdVkbDevice.device, &fb, nullptr, &framebuffer);

    VkCommandBuffer cmd;
    CommandBuffer::init(renderData, cmd);

    VkCommandBufferBeginInfo begin{};
    begin.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    begin.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
    vkBeginCommandBuffer(cmd, &begin);
    {
        Debug::Marker marker(renderData.rdVkbDevice.device, cmd, "Convert Cubemap To PrefilteredMap", Debug::Colors::Green);

        VkImageMemoryBarrier barrier{};
        barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
        barrier.image = prefilteredMapData.image;
        barrier.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        barrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
        barrier.srcAccessMask = 0;
        barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
        barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        barrier.subresourceRange.baseMipLevel = 0;
        barrier.subresourceRange.levelCount = maxMipLevels;
        barrier.subresourceRange.baseArrayLayer = 0;
        barrier.subresourceRange.layerCount = 6;

        vkCmdPipelineBarrier(cmd, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,VK_PIPELINE_STAGE_TRANSFER_BIT,
            0, 0, nullptr, 0, nullptr,
            1, &barrier);

        for (uint32_t mip = 0; mip < maxMipLevels; ++mip)
        {
            uint32_t mipSize = prefilteredMapSize >> mip;
            float roughness = static_cast<float>(mip) / static_cast<float>(maxMipLevels - 1);

            for (uint32_t face = 0; face < 6; ++face)
            {
                struct { uint32_t face; float roughness; } pushData = { face, roughness };
                vkCmdPushConstants(cmd, renderData.rdPrefilterPipelineLayout,
                                VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, 0, sizeof(pushData), &pushData);

                VkRenderPassBeginInfo rp{};
                rp.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
                rp.renderPass = renderData.rdIBLData.rdIBLRenderpass;
                rp.framebuffer = framebuffer;
                rp.renderArea.extent = { mipSize, mipSize };

                VkClearValue clear{};
                clear.color = {{0,0,0,1}};
                rp.clearValueCount = 1;
                rp.pClearValues = &clear;

                vkCmdBeginRenderPass(cmd, &rp, VK_SUBPASS_CONTENTS_INLINE);

                VkViewport viewport{};
                viewport.x = 0.0f;
                viewport.y = 0.0f;
                viewport.width = static_cast<float>(mipSize);
                viewport.height =  static_cast<float>(mipSize);
                viewport.minDepth = 0.f;
                viewport.maxDepth = 1.f;

                VkRect2D scissor{};
                scissor.offset = {0, 0};
                scissor.extent = {mipSize, mipSize};

                vkCmdSetViewport(cmd, 0, 1, &viewport);
                vkCmdSetScissor(cmd, 0, 1, &scissor);

                vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, renderData.rdPrefilterPipeline);

                VkDescriptorSet sets[] = {
                    renderData.rdCaptureUBO.rdUBODescriptorSet,
                    cubemapData.descriptorSet
                };

                vkCmdBindDescriptorSets(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, renderData.rdPrefilterPipelineLayout,
                    0, 2, sets, 0, nullptr);

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
                copy.srcSubresource.mipLevel = 0;
                copy.srcSubresource.baseArrayLayer = 0;
                copy.srcSubresource.layerCount = 1;
                copy.dstSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
                copy.dstSubresource.mipLevel = mip;
                copy.dstSubresource.baseArrayLayer = face;
                copy.dstSubresource.layerCount = 1;
                copy.extent = { mipSize, mipSize, 1 };

                vkCmdCopyImage(cmd, offscreenImage, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
                    prefilteredMapData.image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &copy);

                VkImageMemoryBarrier backToColorBarrier{};
                backToColorBarrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
                backToColorBarrier.image = offscreenImage;
                backToColorBarrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
                backToColorBarrier.newLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
                backToColorBarrier.srcAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
                backToColorBarrier.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
                backToColorBarrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
                backToColorBarrier.subresourceRange.levelCount = 1;
                backToColorBarrier.subresourceRange.layerCount = 1;

                vkCmdPipelineBarrier(cmd, VK_PIPELINE_STAGE_TRANSFER_BIT,
                    VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, 0, 0, nullptr,
                    0, nullptr, 1, &backToColorBarrier);
            }
        }

        VkImageMemoryBarrier prefilteredToSample{};
        prefilteredToSample.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
        prefilteredToSample.image = prefilteredMapData.image;
        prefilteredToSample.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
        prefilteredToSample.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        prefilteredToSample.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
        prefilteredToSample.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
        prefilteredToSample.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        prefilteredToSample.subresourceRange.baseMipLevel = 0;
        prefilteredToSample.subresourceRange.levelCount = maxMipLevels;
        prefilteredToSample.subresourceRange.baseArrayLayer = 0;
        prefilteredToSample.subresourceRange.layerCount = 6;

        vkCmdPipelineBarrier(cmd, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
            0, 0, nullptr, 0, nullptr,
            1, &prefilteredToSample);
    }
    vkEndCommandBuffer(cmd);

    VkSubmitInfo submit{};
    submit.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submit.commandBufferCount = 1;
    submit.pCommandBuffers = &cmd;

    vkQueueSubmit(renderData.rdGraphicsQueue, 1, &submit, VK_NULL_HANDLE);
    vkQueueWaitIdle(renderData.rdGraphicsQueue);

    vkDestroyFramebuffer(renderData.rdVkbDevice.device, framebuffer, nullptr);
    vkDestroyImageView(renderData.rdVkbDevice.device, offscreenImageView, nullptr);
    vmaDestroyImage(renderData.rdAllocator, offscreenImage, offscreenAlloc);

    VkSamplerCreateInfo samplerInfo{};
    samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
    samplerInfo.magFilter = VK_FILTER_LINEAR;
    samplerInfo.minFilter = VK_FILTER_LINEAR;
    samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
    samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
    samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
    samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
    samplerInfo.minLod = 0.0f;
    samplerInfo.maxLod = static_cast<float>(maxMipLevels);

    if (vkCreateSampler(renderData.rdVkbDevice.device, &samplerInfo, nullptr, &prefilteredMapData.sampler) != VK_SUCCESS)
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

    vkCreateDescriptorSetLayout(renderData.rdVkbDevice.device, &layoutInfo, nullptr, &prefilteredMapData.descriptorSetLayout);

    constexpr std::string_view descriptorSetLayoutObjectName = "Descriptor Set Layout IBL PrefilteredMap Data";
    vmaSetAllocationName(renderData.rdAllocator, prefilteredMapData.imageAlloc, descriptorSetLayoutObjectName.data());
    Debug::setObjectName(renderData.rdVkbDevice.device, reinterpret_cast<uint64_t>(prefilteredMapData.descriptorSetLayout),
               VK_OBJECT_TYPE_DESCRIPTOR_SET_LAYOUT, descriptorSetLayoutObjectName.data());

    VkDescriptorPoolSize poolSize{};
    poolSize.type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    poolSize.descriptorCount = 1;

    VkDescriptorPoolCreateInfo poolInfo{};
    poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    poolInfo.poolSizeCount = 1;
    poolInfo.pPoolSizes = &poolSize;
    poolInfo.maxSets = 1;

    if (vkCreateDescriptorPool(renderData.rdVkbDevice.device, &poolInfo, nullptr, &prefilteredMapData.descriptorPool) != VK_SUCCESS)
    {
        Logger::log(1, "%s error: failed to create descriptor pool", __FUNCTION__);
        return false;
    }

    constexpr std::string_view descriptorPoolObjectName = "Descriptor Pool IBL PrefilteredMap Data";
    vmaSetAllocationName(renderData.rdAllocator, prefilteredMapData.imageAlloc, descriptorPoolObjectName.data());
    Debug::setObjectName(renderData.rdVkbDevice.device, reinterpret_cast<uint64_t>(prefilteredMapData.descriptorPool),
                   VK_OBJECT_TYPE_DESCRIPTOR_POOL, descriptorPoolObjectName.data());

    VkDescriptorSetAllocateInfo allocInfoDS{};
    allocInfoDS.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    allocInfoDS.descriptorPool = prefilteredMapData.descriptorPool;
    allocInfoDS.descriptorSetCount = 1;
    allocInfoDS.pSetLayouts = &prefilteredMapData.descriptorSetLayout;

    vkAllocateDescriptorSets(renderData.rdVkbDevice.device, &allocInfoDS, &prefilteredMapData.descriptorSet);

    VkDescriptorImageInfo imageInfoDS{};
    imageInfoDS.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    imageInfoDS.imageView = prefilteredMapData.imageView;
    imageInfoDS.sampler = prefilteredMapData.sampler;

    VkWriteDescriptorSet write{};
    write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    write.dstSet = prefilteredMapData.descriptorSet;
    write.dstBinding = 0;
    write.descriptorCount = 1;
    write.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    write.pImageInfo = &imageInfoDS;

    vkUpdateDescriptorSets(renderData.rdVkbDevice.device, 1, &write, 0, nullptr);

    Logger::log(1, "%s: PrefilteredMap cubemap generated", __FUNCTION__);

    return true;
}

bool Core::Renderer::IBLGenerator::generateBRDFLUT(VkRenderData& renderData, VkTextureData& brdfLutData)
{
    constexpr uint32_t lutSize = 512;

    VkImageCreateInfo imageInfo{};
    imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    imageInfo.imageType = VK_IMAGE_TYPE_2D;
    imageInfo.format = VK_FORMAT_R32G32B32A32_SFLOAT;
    imageInfo.extent = { lutSize, lutSize, 1 };
    imageInfo.mipLevels = 1;
    imageInfo.arrayLayers = 1;
    imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
    imageInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
    imageInfo.usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;

    VmaAllocationCreateInfo allocInfo{};
    allocInfo.usage = VMA_MEMORY_USAGE_GPU_ONLY;

    vmaCreateImage(renderData.rdAllocator, &imageInfo, &allocInfo,
                   &brdfLutData.image, &brdfLutData.imageAlloc, nullptr);

    VkImageViewCreateInfo viewInfo{};
    viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    viewInfo.image = brdfLutData.image;
    viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
    viewInfo.format = imageInfo.format;
    viewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    viewInfo.subresourceRange.levelCount = 1;
    viewInfo.subresourceRange.layerCount = 1;

    vkCreateImageView(renderData.rdVkbDevice.device, &viewInfo, nullptr, &brdfLutData.imageView);

    VkFramebuffer framebuffer;
    VkFramebufferCreateInfo fb{};
    fb.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
    fb.renderPass = renderData.rdIBLData.rdIBLRenderpass;
    fb.attachmentCount = 1;
    fb.pAttachments = &brdfLutData.imageView;
    fb.width = lutSize;
    fb.height = lutSize;
    fb.layers = 1;

    vkCreateFramebuffer(renderData.rdVkbDevice.device, &fb, nullptr, &framebuffer);

    VkCommandBuffer cmd;
    CommandBuffer::init(renderData, cmd);

    VkCommandBufferBeginInfo begin{};
    begin.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    begin.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
    vkBeginCommandBuffer(cmd, &begin);
    {
        Debug::Marker marker(renderData.rdVkbDevice.device, cmd, "Generate BRDF LUT", Debug::Colors::Blue);

        VkImageMemoryBarrier barrier{};
        barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
        barrier.image = brdfLutData.image;
        barrier.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        barrier.newLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
        barrier.srcAccessMask = 0;
        barrier.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
        barrier.subresourceRange = { VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1 };

        vkCmdPipelineBarrier(cmd, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
                             0, 0, nullptr, 0,
                             nullptr, 1, &barrier);

        VkRenderPassBeginInfo rp{};
        rp.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
        rp.renderPass = renderData.rdIBLData.rdIBLRenderpass;
        rp.framebuffer = framebuffer;
        rp.renderArea.extent = { lutSize, lutSize };
        VkClearValue clear = {{{0,0,0,1}}};
        rp.clearValueCount = 1;
        rp.pClearValues = &clear;

        vkCmdBeginRenderPass(cmd, &rp, VK_SUBPASS_CONTENTS_INLINE);

        VkViewport viewport{0.0f, 0.0f, static_cast<float>(lutSize), static_cast<float>(lutSize), 0.0f, 1.0f};
        VkRect2D scissor{{0, 0}, {lutSize, lutSize}};
        vkCmdSetViewport(cmd, 0, 1, &viewport);
        vkCmdSetScissor(cmd, 0, 1, &scissor);

        vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, renderData.rdBRDFLUTPipeline);

        vkCmdDraw(cmd, 3, 1, 0, 0);

        vkCmdEndRenderPass(cmd);

        barrier.oldLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
        barrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        barrier.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
        barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

        vkCmdPipelineBarrier(cmd, VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
                             0, 0, nullptr, 0,
                             nullptr, 1, &barrier);
    }
    vkEndCommandBuffer(cmd);

    VkSubmitInfo submit{};
    submit.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submit.commandBufferCount = 1;
    submit.pCommandBuffers = &cmd;
    vkQueueSubmit(renderData.rdGraphicsQueue, 1, &submit, VK_NULL_HANDLE);
    vkQueueWaitIdle(renderData.rdGraphicsQueue);

    vkDestroyFramebuffer(renderData.rdVkbDevice.device, framebuffer, nullptr);

    VkSamplerCreateInfo samplerInfo{};
    samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
    samplerInfo.magFilter = VK_FILTER_LINEAR;
    samplerInfo.minFilter = VK_FILTER_LINEAR;
    samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
    samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
    samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;

    if (vkCreateSampler(renderData.rdVkbDevice.device, &samplerInfo, nullptr, &brdfLutData.sampler) != VK_SUCCESS)
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

    vkCreateDescriptorSetLayout(renderData.rdVkbDevice.device, &layoutInfo, nullptr, &brdfLutData.descriptorSetLayout);

    constexpr std::string_view descriptorSetLayoutObjectName = "Descriptor Set Layout IBL BRDF LUT Data";
    vmaSetAllocationName(renderData.rdAllocator, brdfLutData.imageAlloc, descriptorSetLayoutObjectName.data());
    Debug::setObjectName(renderData.rdVkbDevice.device, reinterpret_cast<uint64_t>(brdfLutData.descriptorSetLayout),
               VK_OBJECT_TYPE_DESCRIPTOR_SET_LAYOUT, descriptorSetLayoutObjectName.data());

    VkDescriptorPoolSize poolSize{};
    poolSize.type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    poolSize.descriptorCount = 1;

    VkDescriptorPoolCreateInfo poolInfo{};
    poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    poolInfo.poolSizeCount = 1;
    poolInfo.pPoolSizes = &poolSize;
    poolInfo.maxSets = 1;

    if (vkCreateDescriptorPool(renderData.rdVkbDevice.device, &poolInfo, nullptr, &brdfLutData.descriptorPool) != VK_SUCCESS)
    {
        Logger::log(1, "%s error: failed to create descriptor pool", __FUNCTION__);
        return false;
    }

    constexpr std::string_view descriptorPoolObjectName = "Descriptor Pool IBL BRDF LUT Data";
    vmaSetAllocationName(renderData.rdAllocator, brdfLutData.imageAlloc, descriptorPoolObjectName.data());
    Debug::setObjectName(renderData.rdVkbDevice.device, reinterpret_cast<uint64_t>(brdfLutData.descriptorPool),
               VK_OBJECT_TYPE_DESCRIPTOR_POOL, descriptorPoolObjectName.data());

    VkDescriptorSetAllocateInfo allocInfoDS{};
    allocInfoDS.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    allocInfoDS.descriptorPool = brdfLutData.descriptorPool;
    allocInfoDS.descriptorSetCount = 1;
    allocInfoDS.pSetLayouts = &brdfLutData.descriptorSetLayout;

    vkAllocateDescriptorSets(renderData.rdVkbDevice.device, &allocInfoDS, &brdfLutData.descriptorSet);

    VkDescriptorImageInfo imageInfoDS{};
    imageInfoDS.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    imageInfoDS.imageView = brdfLutData.imageView;
    imageInfoDS.sampler = brdfLutData.sampler;

    VkWriteDescriptorSet write{};
    write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    write.dstSet = brdfLutData.descriptorSet;
    write.dstBinding = 0;
    write.descriptorCount = 1;
    write.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    write.pImageInfo = &imageInfoDS;

    vkUpdateDescriptorSets(renderData.rdVkbDevice.device, 1, &write, 0, nullptr);

    Logger::log(1, "%s: BRDF LUT generated successfully", __FUNCTION__);

    return true;
}

void Core::Renderer::IBLGenerator::cleanup(VkRenderData& renderData, IBLData& iblData)
{
    HDRToCubemapRenderpass::cleanup(renderData, renderData.rdIBLData.rdIBLRenderpass);

    Texture::cleanup(renderData, renderData.rdHDRTexture);
    Texture::cleanup(renderData, renderData.rdIBLData.rdBRDFLUT);

    vkDestroyDescriptorSetLayout(renderData.rdVkbDevice.device, iblData.rdSingleCubemapDescriptorLayout, nullptr);

    cleanupCubemapResources(renderData, iblData.rdPrefilterMap);
    cleanupCubemapResources(renderData, iblData.rdIrradianceMap);
    cleanupCubemapResources(renderData, renderData.rdSkyboxData);
}

void Core::Renderer::IBLGenerator::cleanupCubemapResources(VkRenderData& renderData, VkCubemapData& cubemapData)
{
    vkDestroySampler(renderData.rdVkbDevice.device, cubemapData.sampler, nullptr);
    vkDestroyImageView(renderData.rdVkbDevice.device, cubemapData.imageView, nullptr);
    vmaDestroyImage(renderData.rdAllocator, cubemapData.image, cubemapData.imageAlloc);
    vkDestroyDescriptorSetLayout(renderData.rdVkbDevice.device, cubemapData.descriptorSetLayout, nullptr);
    vkDestroyDescriptorPool(renderData.rdVkbDevice.device, cubemapData.descriptorPool, nullptr);
}

bool Core::Renderer::IBLGenerator::createHDRToCubemapPipeline(VkRenderData& renderData)
{
    std::vector layouts = {
        renderData.rdCaptureUBO.rdUBODescriptorLayout,
        renderData.rdHDRTexture.descriptorSetLayout
    };

    VkPushConstantRange pushConstantRange{};
    pushConstantRange.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
    pushConstantRange.offset = 0;
    pushConstantRange.size = sizeof(int);

    VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
    pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    pipelineLayoutInfo.setLayoutCount = static_cast<uint32_t>(layouts.size());
    pipelineLayoutInfo.pSetLayouts = layouts.data();
    pipelineLayoutInfo.pushConstantRangeCount = 1;
    pipelineLayoutInfo.pPushConstantRanges = &pushConstantRange;

    if (vkCreatePipelineLayout(renderData.rdVkbDevice.device, &pipelineLayoutInfo, nullptr,
                               &renderData.rdHDRToCubemapPipelineLayout) != VK_SUCCESS)
    {
        Logger::log(1, "%s error: could not create HDR to Cubemap pipeline layout", __FUNCTION__);
        return false;
    }

    constexpr std::string_view vertexShaderFile = "shaders/equirectangular_to_cubemap.vert.spv";
    constexpr std::string_view fragmentShaderFile = "shaders/equirectangular_to_cubemap.frag.spv";

    PipelineConfig pipelineConfig{};
    pipelineConfig.useVertexInput = VK_FALSE;
    pipelineConfig.enableDepthTest = VK_FALSE;
    pipelineConfig.enableDepthWrite = VK_FALSE;
    pipelineConfig.enableBlending = VK_FALSE;
    pipelineConfig.cullMode = VK_CULL_MODE_NONE;
    pipelineConfig.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
    pipelineConfig.depthCompareOp = VK_COMPARE_OP_ALWAYS;

    if (!Pipeline::init(renderData, renderData.rdHDRToCubemapPipelineLayout, renderData.rdHDRToCubemapPipeline,
                        VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST, vertexShaderFile.data(),
                        fragmentShaderFile.data(), pipelineConfig, renderData.rdHDRToCubemapRenderpass))
    {
        Logger::log(1, "%s error: could not init HDR to Cubemap pipeline\n", __FUNCTION__);
        return false;
    }

    return true;
}

bool Core::Renderer::IBLGenerator::createIrradiancePipeline(VkRenderData& renderData)
{
    std::vector layouts = {
        renderData.rdCaptureUBO.rdUBODescriptorLayout,
        renderData.rdIBLData.rdSingleCubemapDescriptorLayout
    };

    VkPushConstantRange pushConstantRange{};
    pushConstantRange.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
    pushConstantRange.offset = 0;
    pushConstantRange.size = sizeof(int);

    VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
    pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    pipelineLayoutInfo.setLayoutCount = static_cast<uint32_t>(layouts.size());
    pipelineLayoutInfo.pSetLayouts = layouts.data();
    pipelineLayoutInfo.pushConstantRangeCount = 1;
    pipelineLayoutInfo.pPushConstantRanges = &pushConstantRange;

    if (vkCreatePipelineLayout(renderData.rdVkbDevice.device, &pipelineLayoutInfo, nullptr,
                               &renderData.rdIrradiancePipelineLayout) != VK_SUCCESS)
    {
        Logger::log(1, "%s error: could not create irradiance pipeline layout", __FUNCTION__);
        return false;
    }

    constexpr std::string_view vertexShaderFile = "shaders/equirectangular_to_cubemap.vert.spv";
    constexpr std::string_view fragmentShaderFile = "shaders/irradiance_convolution.frag.spv";

    PipelineConfig pipelineConfig{};
    pipelineConfig.useVertexInput = VK_FALSE;
    pipelineConfig.enableDepthTest = VK_FALSE;
    pipelineConfig.enableDepthWrite = VK_FALSE;
    pipelineConfig.enableBlending = VK_FALSE;
    pipelineConfig.cullMode = VK_CULL_MODE_NONE;
    pipelineConfig.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
    pipelineConfig.depthCompareOp = VK_COMPARE_OP_ALWAYS;

    if (!Pipeline::init(renderData, renderData.rdIrradiancePipelineLayout, renderData.rdIrradiancePipeline,
                        VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST, vertexShaderFile.data(),
                        fragmentShaderFile.data(), pipelineConfig, renderData.rdIBLData.rdIBLRenderpass))
    {
        Logger::log(1, "%s error: could not init irradiance pipeline\n", __FUNCTION__);
        return false;
    }

    return true;
}

bool Core::Renderer::IBLGenerator::createPrefilterPipeline(VkRenderData& renderData)
{
    std::vector layouts = {
        renderData.rdCaptureUBO.rdUBODescriptorLayout,
        renderData.rdIBLData.rdSingleCubemapDescriptorLayout
    };

    VkPushConstantRange pushConstantRange{};
    pushConstantRange.stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;
    pushConstantRange.offset = 0;
    pushConstantRange.size = sizeof(uint32_t) + sizeof(float);

    VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
    pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    pipelineLayoutInfo.setLayoutCount = static_cast<uint32_t>(layouts.size());
    pipelineLayoutInfo.pSetLayouts = layouts.data();
    pipelineLayoutInfo.pushConstantRangeCount = 1;
    pipelineLayoutInfo.pPushConstantRanges = &pushConstantRange;

    if (vkCreatePipelineLayout(renderData.rdVkbDevice.device, &pipelineLayoutInfo, nullptr,
                               &renderData.rdPrefilterPipelineLayout) != VK_SUCCESS)
    {
        Logger::log(1, "%s error: could not create prefilter pipeline layout", __FUNCTION__);
        return false;
    }

    constexpr std::string_view vertexShaderFile = "shaders/prefilter.vert.spv";
    constexpr std::string_view fragmentShaderFile = "shaders/prefilter.frag.spv";

    PipelineConfig pipelineConfig{};
    pipelineConfig.useVertexInput = VK_FALSE;
    pipelineConfig.enableDepthTest = VK_FALSE;
    pipelineConfig.enableDepthWrite = VK_FALSE;
    pipelineConfig.enableBlending = VK_FALSE;
    pipelineConfig.cullMode = VK_CULL_MODE_NONE;
    pipelineConfig.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
    pipelineConfig.depthCompareOp = VK_COMPARE_OP_ALWAYS;

    if (!Pipeline::init(renderData, renderData.rdPrefilterPipelineLayout, renderData.rdPrefilterPipeline,
                        VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST, vertexShaderFile.data(),
                        fragmentShaderFile.data(), pipelineConfig, renderData.rdIBLData.rdIBLRenderpass))
    {
        Logger::log(1, "%s error: could not init irradiance pipeline\n", __FUNCTION__);
        return false;
    }

    return true;
}

bool Core::Renderer::IBLGenerator::createBRDFLUTPipeline(VkRenderData& renderData)
{
    VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
    pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;

    if (vkCreatePipelineLayout(renderData.rdVkbDevice.device, &pipelineLayoutInfo, nullptr,
                               &renderData.rdBRDFLUTPipelineLayout) != VK_SUCCESS)
    {
        Logger::log(1, "%s error: could not create BRDF LUT pipeline layout", __FUNCTION__);
        return false;
    }

    constexpr std::string_view vertexShaderFile = "shaders/brdf_lut.vert.spv";
    constexpr std::string_view fragmentShaderFile = "shaders/brdf_lut.frag.spv";

    PipelineConfig pipelineConfig{};
    pipelineConfig.useVertexInput = VK_FALSE;
    pipelineConfig.enableDepthTest = VK_FALSE;
    pipelineConfig.enableDepthWrite = VK_FALSE;
    pipelineConfig.enableBlending = VK_FALSE;
    pipelineConfig.cullMode = VK_CULL_MODE_NONE;
    pipelineConfig.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
    pipelineConfig.depthCompareOp = VK_COMPARE_OP_ALWAYS;

    if (!Pipeline::init(renderData, renderData.rdBRDFLUTPipelineLayout, renderData.rdBRDFLUTPipeline,
                        VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST, vertexShaderFile.data(),
                        fragmentShaderFile.data(), pipelineConfig, renderData.rdIBLData.rdIBLRenderpass))
    {
        Logger::log(1, "%s error: could not init BRDF LUT pipeline\n", __FUNCTION__);
        return false;
    }

    return true;
}
