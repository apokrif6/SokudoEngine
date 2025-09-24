#include "core/tools/Logger.h"
#define VMA_IMPLEMENTATION
#define GLM_ENABLE_EXPERIMENTAL
#if defined(_DEBUG) || !defined(NDEBUG)
#define VMA_DEBUG_DETECT_LEAKS 1
#define VMA_DEBUG_INITIALIZE_ALLOCATIONS 1
#define VMA_DEBUG_DETECT_CORRUPTION 1
#define VMA_DEBUG_LOG_FORMAT(fmt, ...) Logger::log(1, fmt, __VA_ARGS__)
#define VMA_DEBUG_LOG(...) Logger::log(1, __VA_ARGS__)
#endif
#include "vk_mem_alloc.h"
#include "VkRenderer.h"
#include "Framebuffer.h"
#include "Renderpass.h"
#include "core/vk-renderer/pipelines/layouts/PipelineLayout.h"
#include "core/vk-renderer/pipelines/Pipeline.h"
#include "CommandPool.h"
#include "core/vk-renderer/buffers/CommandBuffer.h"
#include "SyncObjects.h"
#include "Texture.h"
#include "core/vk-renderer/buffers/UniformBuffer.h"
#include "imgui.h"
#include "core/vk-renderer/buffers/VertexBuffer.h"
#include "core/events/input-events/MouseMovementEvent.h"
#include "core/utils/ShapeUtils.h"
#include "core/vk-renderer/pipelines/MeshPipeline.h"
#include "core/vk-renderer/pipelines/layouts/MeshPipelineLayout.h"
#include "core/animations/Animator.h"
#include "core/animations/AnimationsUtils.h"
#include <core/events/input-events/MouseLockEvent.h>
#include <algorithm>
#include <glm/gtc/matrix_transform.hpp>
#include "core/vk-renderer/pipelines/DebugSkeletonPipeline.h"
#include "core/vk-renderer/pipelines/layouts/DebugSkeletonPipelineLayout.h"
#include "core/engine/Engine.h"

bool Core::Renderer::VkRenderer::init(const unsigned int width, const unsigned int height)
{
    if (!Core::Engine::getInstance().getRenderData().rdWindow)
    {
        Logger::log(1,
                    "%s error: Can't init Vulkan mCore::Renderer. Core::Engine::getInstance().getRenderData().rdWindow "
                    "is invalid\n",
                    __FUNCTION__);
        return false;
    }

    if (!deviceInit())
    {
        return false;
    }

    if (!initVma())
    {
        return false;
    }

    if (!getQueue())
    {
        return false;
    }

    if (!createSwapchain())
    {
        return false;
    }

    if (!createDepthBuffer())
    {
        return false;
    }

    if (!createCommandPool())
    {
        return false;
    }

    if (!createCommandBuffer())
    {
        return false;
    }

    if (!loadPlaceholderTexture())
    {
        return false;
    }

    if (!createUBO())
    {
        return false;
    }

    if (!createVBO())
    {
        return false;
    }

    if (!createRenderPass())
    {
        return false;
    }

    if (!createPipelineLayout())
    {
        return false;
    }

    if (!createBasicPipeline())
    {
        return false;
    }

    if (!createLinePipeline())
    {
        return false;
    }

    if (!createGridPipeline())
    {
        return false;
    }

    if (!loadMeshWithAssimp())
    {
        return false;
    }

    if (!createFramebuffer())
    {
        return false;
    }

    if (!createSyncObjects())
    {
        return false;
    }

    Core::Engine::getInstance().getRenderData().rdWidth = static_cast<int>(width);
    Core::Engine::getInstance().getRenderData().rdHeight = static_cast<int>(height);

    Logger::log(1, "%s: Vulkan Core::Renderer initialized to %ix%i\n", __FUNCTION__, width, height);
    return true;
}

Core::Renderer::VkRenderer::VkRenderer(GLFWwindow* inWindow)
{
    Core::Engine::getInstance().getRenderData().rdWindow = inWindow;
    mPerspectiveViewMatrices.emplace_back(1.0f);
    mPerspectiveViewMatrices.emplace_back(1.0f);
}

void Core::Renderer::VkRenderer::handleWindowResizeEvents(int width, int height)
{
    Core::Renderer::VkRenderData& renderData = Core::Engine::getInstance().getRenderData();

    vkDeviceWaitIdle(renderData.rdVkbDevice.device);

    renderData.rdWidth = width;
    renderData.rdHeight = height;

    recreateSwapchain();

    vkResetCommandBuffer(renderData.rdCommandBuffer, 0);

    Logger::log(1, "%s: resized window to %ix%i\n", __FUNCTION__, width, height);
}

void Core::Renderer::VkRenderer::onEvent(const Event& event)
{
    VkRenderData& renderData = Core::Engine::getInstance().getRenderData();
    
    if (const auto* mouseMovementEvent = dynamic_cast<const MouseMovementEvent*>(&event))
    {
        renderData.rdViewYaw += static_cast<float>(mouseMovementEvent->deltaX) / 10.f;
        if (renderData.rdViewYaw < 0.f)
        {
            renderData.rdViewYaw += 360.f;
        }
        if (renderData.rdViewYaw >= 360.f)
        {
            renderData.rdViewYaw -= 360.f;
        }

        renderData.rdViewPitch -= static_cast<float>(mouseMovementEvent->deltaY) / 10.f;
        renderData.rdViewPitch = std::min(renderData.rdViewPitch, 89.f);
        renderData.rdViewPitch = std::max(renderData.rdViewPitch, -89.f);
    }
    else if (const auto* mouseLockEvent = dynamic_cast<const MouseLockEvent*>(&event))
    {
        if (mouseLockEvent->isLocked)
        {
            renderData.freeCameraMovement = true;
            glfwSetInputMode(renderData.rdWindow, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
            if (glfwRawMouseMotionSupported())
            {
                glfwSetInputMode(renderData.rdWindow, GLFW_RAW_MOUSE_MOTION,
                                 GLFW_TRUE);
            }
        }
        else
        {
            renderData.freeCameraMovement = false;

            glfwSetInputMode(renderData.rdWindow, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
        }
    }
}

void Core::Renderer::VkRenderer::beginUploadFrame(Core::Renderer::VkRenderData& renderData)
{
    vkWaitForFences(renderData.rdVkbDevice.device, 1, &renderData.rdRenderFence, VK_TRUE, UINT64_MAX);

    vkResetFences(renderData.rdVkbDevice.device, 1, &renderData.rdRenderFence);
    vkResetCommandBuffer(renderData.rdCommandBuffer, 0);

    VkCommandBufferBeginInfo beginInfo{};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

    vkBeginCommandBuffer(renderData.rdCommandBuffer, &beginInfo);
}

void Core::Renderer::VkRenderer::update(VkRenderData& renderData, float deltaTime)
{
    handleCameraMovementKeys();

    mMatrixGenerateTimer.start();

    mPerspectiveViewMatrices.at(0) = mCamera.getViewMatrix(renderData);
    mPerspectiveViewMatrices.at(1) = glm::perspective(glm::radians(static_cast<float>(renderData.rdFieldOfView)),
                                                      static_cast<float>(renderData.rdVkbSwapchain.extent.width) /
                                                          static_cast<float>(renderData.rdVkbSwapchain.extent.height),
                                                      0.01f, 50.0f);

    renderData.rdMatrixGenerateTime = mMatrixGenerateTimer.stop();

    mUploadToVBOTimer.start();
    renderData.rdUploadToVBOTime = mUploadToVBOTimer.stop();

    mUploadToUBOTimer.start();
    UniformBuffer::uploadData(renderData, renderData.rdPerspectiveViewMatrixUBO, mPerspectiveViewMatrices);
    renderData.rdUploadToUBOTime = mUploadToUBOTimer.stop();
}

void Core::Renderer::VkRenderer::endUploadFrame(Core::Renderer::VkRenderData& renderData)
{
    vkEndCommandBuffer(renderData.rdCommandBuffer);

    VkSubmitInfo submitInfo{};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &renderData.rdCommandBuffer;

    if (vkQueueSubmit(renderData.rdGraphicsQueue, 1, &submitInfo, renderData.rdRenderFence) != VK_SUCCESS)
    {
        Logger::log(1, "VkRenderer::endUploadFrame error: failed to submit command buffer\n");
        return;
    }
}

void Core::Renderer::VkRenderer::beginRenderFrame(Core::Renderer::VkRenderData& renderData)
{
    uint32_t imageIndex = 0;
    VkResult result = vkAcquireNextImageKHR(renderData.rdVkbDevice.device, renderData.rdVkbSwapchain.swapchain,
                                            UINT64_MAX, renderData.rdPresentSemaphore, VK_NULL_HANDLE, &imageIndex);
    if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR)
    {
        recreateSwapchain();
        return;
    }

    vkWaitForFences(renderData.rdVkbDevice.device, 1, &renderData.rdRenderFence, VK_TRUE, UINT64_MAX);

    vkResetFences(renderData.rdVkbDevice.device, 1, &renderData.rdRenderFence);

    renderData.rdCurrentImageIndex = imageIndex;

    VkCommandBufferBeginInfo beginInfo{VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO};
    beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

    if (vkBeginCommandBuffer(renderData.rdCommandBuffer, &beginInfo) != VK_SUCCESS)
    {
        Logger::log(1, "VkRenderer::beginRenderFrame - vkBeginCommandBuffer failed");
        return;
    }

    VkClearValue clearValues[2] = {{{{0.f, 0.f, 0.f, 1.0f}}}, {1.0f, 0}};
    VkRenderPassBeginInfo renderPassInfo{VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO};
    renderPassInfo.renderPass = renderData.rdRenderpass;
    renderPassInfo.framebuffer = renderData.rdFramebuffers[imageIndex];
    renderPassInfo.renderArea.extent = renderData.rdVkbSwapchain.extent;
    renderPassInfo.clearValueCount = 2;
    renderPassInfo.pClearValues = clearValues;

    VkViewport viewport{
        0,
        static_cast<float>(renderData.rdVkbSwapchain.extent.height),
        static_cast<float>(renderData.rdVkbSwapchain.extent.width),
        -static_cast<float>(renderData.rdVkbSwapchain.extent.height),
        0.0f,
        1.0f
    };

    VkRect2D scissor{{0, 0}, renderData.rdVkbSwapchain.extent};

    vkCmdBeginRenderPass(renderData.rdCommandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);
    vkCmdSetViewport(renderData.rdCommandBuffer, 0, 1, &viewport);
    vkCmdSetScissor(renderData.rdCommandBuffer, 0, 1, &scissor);
}

bool Core::Renderer::VkRenderer::draw(VkRenderData& renderData)
{
    vkCmdBindPipeline(renderData.rdCommandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, renderData.rdLinePipeline);

    vkCmdBindDescriptorSets(renderData.rdCommandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, renderData.rdPipelineLayout, 0,
                            1, &renderData.rdPlaceholderTexture.texTextureDescriptorSet, 0, nullptr);
    vkCmdBindDescriptorSets(renderData.rdCommandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, renderData.rdPipelineLayout, 1,
                            1, &renderData.rdPerspectiveViewMatrixUBO.rdUBODescriptorSet, 0, nullptr);

    VkDeviceSize offset = 0;
    vkCmdBindVertexBuffers(renderData.rdCommandBuffer, 0, 1, &renderData.rdVertexBufferData.rdVertexBuffer, &offset);

    vkCmdBindPipeline(renderData.rdCommandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, renderData.rdGridPipeline);
    vkCmdDraw(renderData.rdCommandBuffer, 6, 1, 0, 0);

    return true;
}

void Core::Renderer::VkRenderer::endRenderFrame(Core::Renderer::VkRenderData& renderData)
{
    vkCmdEndRenderPass(renderData.rdCommandBuffer);

    if (vkEndCommandBuffer(renderData.rdCommandBuffer) != VK_SUCCESS)
    {
        Logger::log(1, "VkRenderer::endRenderFrame - vkEndCommandBuffer failed");
        return;
    }

    VkSubmitInfo submitInfo{VK_STRUCTURE_TYPE_SUBMIT_INFO};
    VkPipelineStageFlags waitStage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    submitInfo.waitSemaphoreCount = 1;
    submitInfo.pWaitSemaphores = &renderData.rdPresentSemaphore;
    submitInfo.pWaitDstStageMask = &waitStage;
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &renderData.rdCommandBuffer;
    submitInfo.signalSemaphoreCount = 1;
    submitInfo.pSignalSemaphores = &renderData.rdRenderSemaphore;

    if (vkQueueSubmit(renderData.rdGraphicsQueue, 1, &submitInfo, renderData.rdRenderFence) != VK_SUCCESS)
    {
        Logger::log(1, "VkRenderer::endRenderFrame - vkQueueSubmit failed");
        return;
    }

    VkPresentInfoKHR presentInfo{VK_STRUCTURE_TYPE_PRESENT_INFO_KHR};
    presentInfo.waitSemaphoreCount = 1;
    presentInfo.pWaitSemaphores = &renderData.rdRenderSemaphore;
    presentInfo.swapchainCount = 1;
    presentInfo.pSwapchains = &renderData.rdVkbSwapchain.swapchain;
    presentInfo.pImageIndices = &renderData.rdCurrentImageIndex;

    vkQueuePresentKHR(renderData.rdPresentQueue, &presentInfo);
}

void Core::Renderer::VkRenderer::cleanup(VkRenderData& renderData)
{
    Core::Renderer::SyncObjects::cleanup(renderData);

    Core::Renderer::CommandBuffer::cleanup(renderData, Core::Engine::getInstance().getRenderData().rdCommandBuffer);
    Core::Renderer::CommandPool::cleanup(renderData);

    Core::Renderer::Framebuffer::cleanup(renderData);

    Core::Renderer::MeshPipeline::cleanup(renderData, renderData.rdMeshPipeline);
    Core::Renderer::DebugSkeletonPipeline::cleanup(renderData,
                                                   Core::Engine::getInstance().getRenderData().rdDebugSkeletonPipeline);
    Core::Renderer::Pipeline::cleanup(renderData, Core::Engine::getInstance().getRenderData().rdGridPipeline);
    Core::Renderer::Pipeline::cleanup(renderData, renderData.rdLinePipeline);
    Core::Renderer::Pipeline::cleanup(renderData, renderData.rdBasicPipeline);

    Core::Renderer::PipelineLayout::cleanup(renderData, renderData.rdPipelineLayout);
    Core::Renderer::MeshPipelineLayout::cleanup(renderData, renderData.rdMeshPipelineLayout);
    Core::Renderer::DebugSkeletonPipelineLayout::cleanup(renderData, renderData.rdDebugSkeletonPipelineLayout);

    Core::Renderer::Renderpass::cleanup(renderData);

    Core::Renderer::UniformBuffer::cleanup(renderData, renderData.rdPerspectiveViewMatrixUBO);
    Core::Renderer::VertexBuffer::cleanup(renderData, renderData.rdVertexBufferData);

    Core::Renderer::Texture::cleanup(renderData, renderData.rdPlaceholderTexture);

    vkDestroyImageView(renderData.rdVkbDevice.device, renderData.rdDepthImageView, nullptr);
    vmaDestroyImage(renderData.rdAllocator, renderData.rdDepthImage,
                    Core::Engine::getInstance().getRenderData().rdDepthImageAlloc);

    vmaDestroyAllocator(renderData.rdAllocator);

    renderData.rdVkbSwapchain.destroy_image_views(renderData.rdSwapchainImageViews);
    vkb::destroy_swapchain(renderData.rdVkbSwapchain);

    vkb::destroy_device(renderData.rdVkbDevice);
    vkb::destroy_surface(renderData.rdVkbInstance.instance, mSurface);
    vkb::destroy_instance(renderData.rdVkbInstance);

    Logger::log(1, "%s: Vulkan Core::Renderer destroyed\n", __FUNCTION__);
}

bool Core::Renderer::VkRenderer::deviceInit()
{
    vkb::InstanceBuilder instBuild;
    auto instRet = instBuild.use_default_debug_messenger()
                       .request_validation_layers()
                       .set_minimum_instance_version(VKB_VK_API_VERSION_1_1)
                       .require_api_version(VKB_VK_API_VERSION_1_3)
                       .build();
    if (!instRet)
    {
        Logger::log(1, "%s error: could not build vkb instance\n", __FUNCTION__);
        return false;
    }
    Core::Engine::getInstance().getRenderData().rdVkbInstance = instRet.value();

    VkResult result = VK_ERROR_UNKNOWN;
    result = glfwCreateWindowSurface(Core::Engine::getInstance().getRenderData().rdVkbInstance,
                                     Core::Engine::getInstance().getRenderData().rdWindow, nullptr, &mSurface);
    if (result != VK_SUCCESS)
    {
        Logger::log(1, "%s error: Could not create Vulkan surface\n", __FUNCTION__);
        return false;
    }

    /* just get the first available device */
    vkb::PhysicalDeviceSelector physicalDevSel{Core::Engine::getInstance().getRenderData().rdVkbInstance};
    auto firstPhysicalDevSelRet = physicalDevSel.set_surface(mSurface).select();
    if (!firstPhysicalDevSelRet)
    {
        Logger::log(1, "%s error: could not get physical devices\n", __FUNCTION__);
        return false;
    }

    VkPhysicalDeviceFeatures physicalFeatures;
    vkGetPhysicalDeviceFeatures(firstPhysicalDevSelRet.value(), &physicalFeatures);

    auto secondPhysicalDevSelRet =
        physicalDevSel.set_surface(mSurface).set_required_features(physicalFeatures).select();
    if (!secondPhysicalDevSelRet)
    {
        Logger::log(1, "%s error: could not get physical devices\n", __FUNCTION__);
        return false;
    }

    Core::Engine::getInstance().getRenderData().rdVkbPhysicalDevice = secondPhysicalDevSelRet.value();

    Logger::log(1, "%s: found physical device '%s'\n", __FUNCTION__,
                Core::Engine::getInstance().getRenderData().rdVkbPhysicalDevice.name.c_str());

    mMinUniformBufferOffsetAlignment = Core::Engine::getInstance()
                                           .getRenderData()
                                           .rdVkbPhysicalDevice.properties.limits.minUniformBufferOffsetAlignment;
    Logger::log(1, "%s: the physical device as a minimal uniform buffer offset of %i bytes\n", __FUNCTION__,
                mMinUniformBufferOffsetAlignment);

    vkb::DeviceBuilder devBuilder{Core::Engine::getInstance().getRenderData().rdVkbPhysicalDevice};
    auto devBuilderRet = devBuilder.build();
    if (!devBuilderRet)
    {
        Logger::log(1, "%s error: could not get devices\n", __FUNCTION__);
        return false;
    }
    Core::Engine::getInstance().getRenderData().rdVkbDevice = devBuilderRet.value();

    return true;
}

bool Core::Renderer::VkRenderer::getQueue()
{
    auto graphQueueRet = Core::Engine::getInstance().getRenderData().rdVkbDevice.get_queue(vkb::QueueType::graphics);
    if (!graphQueueRet.has_value())
    {
        Logger::log(1, "%s error: could not get graphics queue\n", __FUNCTION__);
        return false;
    }
    Core::Engine::getInstance().getRenderData().rdGraphicsQueue = graphQueueRet.value();

    auto presentQueueRet = Core::Engine::getInstance().getRenderData().rdVkbDevice.get_queue(vkb::QueueType::present);
    if (!presentQueueRet.has_value())
    {
        Logger::log(1, "%s error: could not get present queue\n", __FUNCTION__);
        return false;
    }
    Core::Engine::getInstance().getRenderData().rdPresentQueue = presentQueueRet.value();

    return true;
}

bool Core::Renderer::VkRenderer::createSwapchain()
{
    vkb::SwapchainBuilder swapChainBuild{Core::Engine::getInstance().getRenderData().rdVkbDevice};

    glfwGetFramebufferSize(Core::Engine::getInstance().getRenderData().rdWindow,
                           &Core::Engine::getInstance().getRenderData().rdWidth,
                           &Core::Engine::getInstance().getRenderData().rdHeight);

    auto swapChainBuildRet =
        swapChainBuild.set_old_swapchain(Core::Engine::getInstance().getRenderData().rdVkbSwapchain)
            .set_desired_present_mode(VK_PRESENT_MODE_FIFO_KHR)
            .set_desired_extent(Core::Engine::getInstance().getRenderData().rdWidth,
                                Core::Engine::getInstance().getRenderData().rdHeight)
            .build();
    if (!swapChainBuildRet)
    {
        Logger::log(1, "%s error: could not init swapchain\n", __FUNCTION__);
        return false;
    }

    vkb::destroy_swapchain(Core::Engine::getInstance().getRenderData().rdVkbSwapchain);
    Core::Engine::getInstance().getRenderData().rdVkbSwapchain = swapChainBuildRet.value();

    return true;
}

bool Core::Renderer::VkRenderer::createDepthBuffer()
{
    const VkExtent3D depthImageExtent = {Core::Engine::getInstance().getRenderData().rdVkbSwapchain.extent.width,
                                         Core::Engine::getInstance().getRenderData().rdVkbSwapchain.extent.height, 1};

    Core::Engine::getInstance().getRenderData().rdDepthFormat = VK_FORMAT_D32_SFLOAT;

    VkImageCreateInfo depthImageInfo{};
    depthImageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    depthImageInfo.imageType = VK_IMAGE_TYPE_2D;
    depthImageInfo.format = Core::Engine::getInstance().getRenderData().rdDepthFormat;
    depthImageInfo.extent = depthImageExtent;
    depthImageInfo.mipLevels = 1;
    depthImageInfo.arrayLayers = 1;
    depthImageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
    depthImageInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
    depthImageInfo.usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;

    VmaAllocationCreateInfo depthAllocInfo{};
    depthAllocInfo.usage = VMA_MEMORY_USAGE_GPU_ONLY;
    depthAllocInfo.requiredFlags = static_cast<VkMemoryPropertyFlags>(VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

    if (vmaCreateImage(Core::Engine::getInstance().getRenderData().rdAllocator, &depthImageInfo, &depthAllocInfo,
                       &Core::Engine::getInstance().getRenderData().rdDepthImage,
                       &Core::Engine::getInstance().getRenderData().rdDepthImageAlloc, nullptr) != VK_SUCCESS)
    {
        Logger::log(1, "%s error: could not allocate depth buffer memory\n", __FUNCTION__);
        return false;
    }

    VkImageViewCreateInfo depthImageViewInfo{};
    depthImageViewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    depthImageViewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
    depthImageViewInfo.image = Core::Engine::getInstance().getRenderData().rdDepthImage;
    depthImageViewInfo.format = Core::Engine::getInstance().getRenderData().rdDepthFormat;
    depthImageViewInfo.subresourceRange.baseMipLevel = 0;
    depthImageViewInfo.subresourceRange.levelCount = 1;
    depthImageViewInfo.subresourceRange.baseArrayLayer = 0;
    depthImageViewInfo.subresourceRange.layerCount = 1;
    depthImageViewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;

    if (vkCreateImageView(Core::Engine::getInstance().getRenderData().rdVkbDevice.device, &depthImageViewInfo, nullptr,
                          &Core::Engine::getInstance().getRenderData().rdDepthImageView) != VK_SUCCESS)
    {
        Logger::log(1, "%s error: could not create depth buffer image view\n", __FUNCTION__);
        return false;
    }
    return true;
}

bool Core::Renderer::VkRenderer::recreateSwapchain()
{
    while (Core::Engine::getInstance().getRenderData().rdWidth == 0 ||
           Core::Engine::getInstance().getRenderData().rdHeight == 0)
    {
        glfwGetFramebufferSize(Core::Engine::getInstance().getRenderData().rdWindow,
                               &Core::Engine::getInstance().getRenderData().rdWidth,
                               &Core::Engine::getInstance().getRenderData().rdHeight);
        glfwWaitEvents();
    }

    vkDeviceWaitIdle(Core::Engine::getInstance().getRenderData().rdVkbDevice.device);

    /* cleanup */
    Core::Renderer::Framebuffer::cleanup(Core::Engine::getInstance().getRenderData());
    vkDestroyImageView(Core::Engine::getInstance().getRenderData().rdVkbDevice.device,
                       Core::Engine::getInstance().getRenderData().rdDepthImageView, nullptr);
    vmaDestroyImage(Core::Engine::getInstance().getRenderData().rdAllocator,
                    Core::Engine::getInstance().getRenderData().rdDepthImage,
                    Core::Engine::getInstance().getRenderData().rdDepthImageAlloc);

    Core::Engine::getInstance().getRenderData().rdVkbSwapchain.destroy_image_views(
        Core::Engine::getInstance().getRenderData().rdSwapchainImageViews);

    /* and recreate */
    if (!createSwapchain())
    {
        Logger::log(1, "%s error: could not recreate swapchain\n", __FUNCTION__);
        return false;
    }

    if (!createDepthBuffer())
    {
        Logger::log(1, "%s error: could not recreate depth buffer\n", __FUNCTION__);
        return false;
    }

    if (!createFramebuffer())
    {
        Logger::log(1, "%s error: could not recreate framebuffers\n", __FUNCTION__);
        return false;
    }

    return true;
}

bool Core::Renderer::VkRenderer::createUBO()
{
    size_t matrixSize = mPerspectiveViewMatrices.size() * sizeof(glm::mat4);

    if (!Core::Renderer::UniformBuffer::init(Core::Engine::getInstance().getRenderData(),
                                             Core::Engine::getInstance().getRenderData().rdPerspectiveViewMatrixUBO,
                                             matrixSize, "PerspectiveViewMatrixUBO"))
    {
        Logger::log(1, "%s error: could not create uniform buffers\n", __FUNCTION__);
        return false;
    }
    return true;
}

bool Core::Renderer::VkRenderer::createVBO()
{
    if (!Core::Renderer::VertexBuffer::init(Core::Engine::getInstance().getRenderData(),
                                            Core::Engine::getInstance().getRenderData().rdVertexBufferData,
                                            VertexBufferSize, "MainVBO"))
    {
        Logger::log(1, "%s error: could not create VBO\n", __FUNCTION__);
        return false;
    }
    return true;
}

bool Core::Renderer::VkRenderer::createRenderPass()
{
    if (!Core::Renderer::Renderpass::init(Core::Engine::getInstance().getRenderData()))
    {
        Logger::log(1, "%s error: could not init renderpass\n", __FUNCTION__);
        return false;
    }
    return true;
}

bool Core::Renderer::VkRenderer::createPipelineLayout()
{
    if (!Core::Renderer::PipelineLayout::init(Core::Engine::getInstance().getRenderData(),
                                              Core::Engine::getInstance().getRenderData().rdPlaceholderTexture,
                                              Core::Engine::getInstance().getRenderData().rdPipelineLayout))
    {
        Logger::log(1, "%s error: could not init pipeline layout\n", __FUNCTION__);
        return false;
    }
    return true;
}

bool Core::Renderer::VkRenderer::createBasicPipeline()
{
    const std::string vertexShaderFile = "shaders/basic.vert.spv";
    const std::string fragmentShaderFile = "shaders/basic.frag.spv";
    if (!Pipeline::init(Core::Engine::getInstance().getRenderData(),
                        Core::Engine::getInstance().getRenderData().rdPipelineLayout,
                        Core::Engine::getInstance().getRenderData().rdBasicPipeline,
                        VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST, vertexShaderFile, fragmentShaderFile))
    {
        Logger::log(1, "%s error: could not init pipeline\n", __FUNCTION__);
        return false;
    }
    return true;
}

bool Core::Renderer::VkRenderer::createLinePipeline()
{
    const std::string vertexShaderFile = "shaders/line.vert.spv";
    const std::string fragmentShaderFile = "shaders/line.frag.spv";
    if (!Pipeline::init(Core::Engine::getInstance().getRenderData(),
                        Core::Engine::getInstance().getRenderData().rdPipelineLayout,
                        Core::Engine::getInstance().getRenderData().rdLinePipeline, VK_PRIMITIVE_TOPOLOGY_LINE_LIST,
                        vertexShaderFile, fragmentShaderFile))
    {
        Logger::log(1, "%s error: could not init line shader pipeline\n", __FUNCTION__);
        return false;
    }
    return true;
}

bool Core::Renderer::VkRenderer::createGridPipeline()
{
    const std::string vertexShaderFile = "shaders/grid.vert.spv";
    const std::string fragmentShaderFile = "shaders/grid.frag.spv";
    if (!Pipeline::init(Core::Engine::getInstance().getRenderData(),
                        Core::Engine::getInstance().getRenderData().rdPipelineLayout,
                        Core::Engine::getInstance().getRenderData().rdGridPipeline, VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST,
                        vertexShaderFile, fragmentShaderFile))
    {
        Logger::log(1, "%s error: could not init grid shader pipeline\n", __FUNCTION__);
        return false;
    }
    return true;
}

bool Core::Renderer::VkRenderer::createFramebuffer()
{
    if (!Core::Renderer::Framebuffer::init(Core::Engine::getInstance().getRenderData()))
    {
        Logger::log(1, "%s error: could not init framebuffer\n", __FUNCTION__);
        return false;
    }
    return true;
}

bool Core::Renderer::VkRenderer::createCommandPool()
{
    if (!Core::Renderer::CommandPool::init(Core::Engine::getInstance().getRenderData()))
    {
        Logger::log(1, "%s error: could not create command pool\n", __FUNCTION__);
        return false;
    }
    return true;
}

bool Core::Renderer::VkRenderer::createCommandBuffer()
{
    if (!Core::Renderer::CommandBuffer::init(Core::Engine::getInstance().getRenderData(),
                                             Core::Engine::getInstance().getRenderData().rdCommandBuffer))
    {
        Logger::log(1, "%s error: could not create command buffers\n", __FUNCTION__);
        return false;
    }
    return true;
}

bool Core::Renderer::VkRenderer::createSyncObjects()
{
    if (!Core::Renderer::SyncObjects::init(Core::Engine::getInstance().getRenderData()))
    {
        Logger::log(1, "%s error: could not create sync objects\n", __FUNCTION__);
        return false;
    }

    return true;
}

bool Core::Renderer::VkRenderer::loadPlaceholderTexture()
{
    const std::string textureFileName = "assets/textures/placeholder_sampler.png";
    std::future<bool> textureLoadFuture = Core::Renderer::Texture::loadTexture(
        Core::Engine::getInstance().getRenderData(), Core::Engine::getInstance().getRenderData().rdPlaceholderTexture,
        textureFileName);
    if (!textureLoadFuture.get())
    {
        Logger::log(1, "%s error: could not load texture\n", __FUNCTION__);
        return false;
    }

    return true;
}

bool Core::Renderer::VkRenderer::initVma()
{
    VmaAllocatorCreateInfo allocatorInfo{};
    allocatorInfo.physicalDevice = Core::Engine::getInstance().getRenderData().rdVkbPhysicalDevice.physical_device;
    allocatorInfo.device = Core::Engine::getInstance().getRenderData().rdVkbDevice.device;
    allocatorInfo.instance = Core::Engine::getInstance().getRenderData().rdVkbInstance.instance;
    if (vmaCreateAllocator(&allocatorInfo, &Core::Engine::getInstance().getRenderData().rdAllocator) != VK_SUCCESS)
    {
        Logger::log(1, "%s error: could not init VMA\n", __FUNCTION__);
        return false;
    }

    return true;
}

bool Core::Renderer::VkRenderer::loadMeshWithAssimp()
{
    const std::string modelFileName = "assets/mixamo/models/FemaleModel.fbx";
    const std::vector<std::string> animationsFileNames = {
         "assets/mixamo/animations/StandingIdleAnimation.fbx",
         "assets/mixamo/animations/AngryAnimation.fbx",
         "assets/mixamo/animations/BoxingAnimation.fbx",
         "assets/mixamo/animations/HipHopDancingAnimation.fbx",
         "assets/mixamo/animations/StandingReactDeathBackwardAnimation.fbx"
    };
    Core::Utils::MeshData primitiveMeshData =
        Core::Utils::loadMeshFromFile(modelFileName, Core::Engine::getInstance().getRenderData());
    for (const auto& animationFileName : animationsFileNames)
    {
        auto animation = Core::Animations::AnimationsUtils::loadAnimationFromFile(animationFileName);
        if (animation.name.empty())
        {
            Logger::log(1, "%s error: could not load animation from file %s", __FUNCTION__, animationFileName.c_str());
            return false;
        }
        primitiveMeshData.animations.push_back(animation);
    }
    if (!Core::Renderer::MeshPipelineLayout::init(Core::Engine::getInstance().getRenderData(),
                                                  Core::Engine::getInstance().getRenderData().rdMeshPipelineLayout))
    {
        Logger::log(1, "%s error: could not init mesh pipeline layout\n", __FUNCTION__);
        return false;
    }

    const std::string vertexShaderFile = "shaders/primitive.vert.spv";
    const std::string fragmentShaderFile = "shaders/primitive.frag.spv";
    if (!MeshPipeline::init(Core::Engine::getInstance().getRenderData(),
                            Core::Engine::getInstance().getRenderData().rdMeshPipelineLayout,
                            Core::Engine::getInstance().getRenderData().rdMeshPipeline,
                            VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST, vertexShaderFile, fragmentShaderFile))
    {
        Logger::log(1, "%s error: could not init mesh pipeline\n", __FUNCTION__);
        return false;
    }

    createDebugSkeletonPipelineLayout();
    createDebugSkeletonPipeline();

    auto testMeshOne = std::make_shared<Mesh>("TestMesh_1", primitiveMeshData.skeleton);
    testMeshOne->setupAnimations(primitiveMeshData.animations);
    testMeshOne->initDebugSkeleton(Core::Engine::getInstance().getRenderData());
    testMeshOne->getTransform().position = {1, 0, 1};
    testMeshOne->setMeshFilePath(modelFileName);
    testMeshOne->setAnimationFiles(animationsFileNames);

    auto testMeshTwo = std::make_shared<Mesh>("TestMesh_2", primitiveMeshData.skeleton);
    testMeshTwo->setupAnimations(primitiveMeshData.animations);
    testMeshTwo->initDebugSkeleton(Core::Engine::getInstance().getRenderData());
    testMeshTwo->getTransform().position = {-1, 0, 1};
    testMeshTwo->setMeshFilePath(modelFileName);
    testMeshTwo->setAnimationFiles(animationsFileNames);

    auto testMeshThree = std::make_shared<Mesh>("TestMesh_3", primitiveMeshData.skeleton);
    testMeshThree->setupAnimations(primitiveMeshData.animations);
    testMeshThree->initDebugSkeleton(Core::Engine::getInstance().getRenderData());
    testMeshThree->getTransform().position = {1, 0, -1};
    testMeshThree->setMeshFilePath(modelFileName);
    testMeshThree->setAnimationFiles(animationsFileNames);

    auto testMeshFour = std::make_shared<Mesh>("TestMesh_4", primitiveMeshData.skeleton);
    testMeshFour->setupAnimations(primitiveMeshData.animations);
    testMeshFour->initDebugSkeleton(Core::Engine::getInstance().getRenderData());
    testMeshFour->getTransform().position = {-1, 0, -1};
    testMeshFour->setMeshFilePath(modelFileName);
    testMeshFour->setAnimationFiles(animationsFileNames);

    Core::Engine::getInstance().getSystem<Scene::Scene>()->addObject(testMeshOne);
    Core::Engine::getInstance().getSystem<Scene::Scene>()->addObject(testMeshTwo);
    Core::Engine::getInstance().getSystem<Scene::Scene>()->addObject(testMeshThree);
    Core::Engine::getInstance().getSystem<Scene::Scene>()->addObject(testMeshFour);

    for (auto& primitive : primitiveMeshData.primitives)
    {
        // TODO
        // should be refactored when multiple texture types will be supported
        // textures should be passed as param, so there will be parameter with whole map or nullptr
        Core::Renderer::VkTextureData primitiveTexture;
        auto foundDiffuseTexture = primitive.textures.find(aiTextureType_DIFFUSE);
        if (foundDiffuseTexture != primitive.textures.end())
        {
            primitiveTexture = foundDiffuseTexture->second;
        }
        testMeshOne->addPrimitive(primitive.vertices, primitive.indices, primitiveTexture,
                                  Core::Engine::getInstance().getRenderData(), primitive.material, primitive.bones);
        testMeshTwo->addPrimitive(primitive.vertices, primitive.indices, primitiveTexture,
                                  Core::Engine::getInstance().getRenderData(), primitive.material, primitive.bones);
        testMeshThree->addPrimitive(primitive.vertices, primitive.indices, primitiveTexture,
                                  Core::Engine::getInstance().getRenderData(), primitive.material, primitive.bones);
        testMeshFour->addPrimitive(primitive.vertices, primitive.indices, primitiveTexture,
                                  Core::Engine::getInstance().getRenderData(), primitive.material, primitive.bones);
    }

    return true;
}

bool Core::Renderer::VkRenderer::createDebugSkeletonPipelineLayout()
{
    if (!Core::Renderer::DebugSkeletonPipelineLayout::init(
            Core::Engine::getInstance().getRenderData(),
            Core::Engine::getInstance().getRenderData().rdDebugSkeletonPipelineLayout))
    {
        Logger::log(1, "%s error: could not init debug skeleton pipeline layout\n", __FUNCTION__);
        return false;
    }

    return true;
}

bool Core::Renderer::VkRenderer::createDebugSkeletonPipeline()
{
    const std::string vertexShaderFile = "shaders/debug_line.vert.spv";
    const std::string fragmentShaderFile = "shaders/debug_line.frag.spv";
    if (!DebugSkeletonPipeline::init(Core::Engine::getInstance().getRenderData(),
                                     Core::Engine::getInstance().getRenderData().rdDebugSkeletonPipelineLayout,
                                     Core::Engine::getInstance().getRenderData().rdDebugSkeletonPipeline,
                                     VK_PRIMITIVE_TOPOLOGY_LINE_LIST, vertexShaderFile, fragmentShaderFile))
    {
        Logger::log(1, "%s error: could not init debug skeleton shader pipeline\n", __FUNCTION__);
        return false;
    }

    return true;
}

void Core::Renderer::VkRenderer::handleWindowMoveEvents(int xPosition, int yPosition)
{
    Logger::log(1, "%s: Core::Engine::getInstance().getRenderData().rdWindow has been moved to %i/%i\n", __FUNCTION__,
                xPosition, yPosition);
}

void Core::Renderer::VkRenderer::handleWindowMinimizedEvents(int minimized)
{
    if (minimized)
    {
        Logger::log(1, "%s: Core::Engine::getInstance().getRenderData().rdWindow has been minimized\n", __FUNCTION__);
    }
    else
    {
        Logger::log(1, "%s: Core::Engine::getInstance().getRenderData().rdWindow has been restored\n", __FUNCTION__);
    }
}

void Core::Renderer::VkRenderer::handleWindowMaximizedEvents(int maximized)
{
    if (maximized)
    {
        Logger::log(1, "%s: Core::Engine::getInstance().getRenderData().rdWindow has been maximized\n", __FUNCTION__);
    }
    else
    {
        Logger::log(1, "%s: Core::Engine::getInstance().getRenderData().rdWindow has been restored\n", __FUNCTION__);
    }
}

void Core::Renderer::VkRenderer::handleWindowCloseEvents()
{
    Logger::log(1, "%s: Core::Engine::getInstance().getRenderData().rdWindow has been closed\n", __FUNCTION__);
}

void Core::Renderer::VkRenderer::handleMouseEnterLeaveEvents(int enter)
{
    if (enter)
    {
        Logger::log(1, "%s: Mouse entered window\n", __FUNCTION__);
    }
    else
    {
        Logger::log(1, "%s: Mouse left window\n", __FUNCTION__);
    }
}

void Core::Renderer::VkRenderer::handleCameraMovementKeys()
{
    ImGuiIO& io = ImGui::GetIO();
    if (io.WantCaptureKeyboard)
    {
        return;
    }

    VkRenderData& renderData = Core::Engine::getInstance().getRenderData();
    if (!renderData.freeCameraMovement)
    {
        renderData.rdMoveForward = 0.f;
        renderData.rdMoveRight = 0.f;
        renderData.rdMoveUp = 0.f;

        return;
    }

    renderData.rdMoveForward = 0.f;
    if (glfwGetKey(renderData.rdWindow, GLFW_KEY_W) == GLFW_PRESS)
    {
        renderData.rdMoveForward += 1.f;
    }
    if (glfwGetKey(renderData.rdWindow, GLFW_KEY_S) == GLFW_PRESS)
    {
        renderData.rdMoveForward -= 1.f;
    }

    renderData.rdMoveRight = 0.f;
    if (glfwGetKey(renderData.rdWindow, GLFW_KEY_D) == GLFW_PRESS)
    {
        renderData.rdMoveRight += 1;
    }
    if (glfwGetKey(renderData.rdWindow, GLFW_KEY_A) == GLFW_PRESS)
    {
        renderData.rdMoveRight -= 1.f;
    }

    renderData.rdMoveUp = 0.f;
    if (glfwGetKey(renderData.rdWindow, GLFW_KEY_E) == GLFW_PRESS)
    {
        renderData.rdMoveUp += 1.f;
    }
    if (glfwGetKey(renderData.rdWindow, GLFW_KEY_Q) == GLFW_PRESS)
    {
        renderData.rdMoveUp -= 1.f;
    }
}
