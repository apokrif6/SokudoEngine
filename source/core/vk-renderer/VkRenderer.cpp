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
#include "core/vk-renderer/pipelines/layouts/MeshPipelineLayout.h"
#include "core/animations/Animator.h"
#include <core/events/input-events/MouseLockEvent.h>
#include <algorithm>
#include <glm/gtc/matrix_transform.hpp>
#include "core/vk-renderer/pipelines/DebugSkeletonPipeline.h"
#include "core/vk-renderer/pipelines/layouts/DebugSkeletonPipelineLayout.h"
#include "core/vk-renderer/Cubemap.h"
#include "core/engine/Engine.h"
#include "core/vk-renderer/viewport/ViewportRenderpass.h"
#include "core/vk-renderer/viewport/ViewportTarget.h"
#include "imgui_impl_vulkan.h"
#include "cubemap_generator/HDRToCubemapRenderpass.h"

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

    if (!createDummyBonesTransformUBO())
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

    if (!createViewportRenderpass())
    {
        return false;
    }

    if (!createPipelineLayout())
    {
        return false;
    }

    if (!createGridPipeline())
    {
        return false;
    }

    if (!loadSkybox())
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
                glfwSetInputMode(renderData.rdWindow, GLFW_RAW_MOUSE_MOTION, GLFW_TRUE);
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

void Core::Renderer::VkRenderer::draw(VkRenderData& renderData)
{
    if (renderData.shouldDrawSkybox)
    {
        drawSkybox();
    }

    if (renderData.shouldDrawGrid)
    {
        drawGrid();
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

    vkBeginCommandBuffer(renderData.rdCommandBuffer, &beginInfo);
}

void Core::Renderer::VkRenderer::beginOffscreenRenderPass(Core::Renderer::VkRenderData& renderData)
{
    VkClearValue clearValues[2] = {{{{0.f, 0.f, 0.f, 1.0f}}}, {1.0f, 0}};
    VkRenderPassBeginInfo offscreenRenderPassInfo{VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO};
    offscreenRenderPassInfo.renderPass = renderData.rdViewportTarget.renderpass;
    offscreenRenderPassInfo.framebuffer = renderData.rdViewportTarget.framebuffer;
    offscreenRenderPassInfo.renderArea.extent = {static_cast<uint32_t>(renderData.rdViewportTarget.size.x),
                                                 static_cast<uint32_t>(renderData.rdViewportTarget.size.y)};
    offscreenRenderPassInfo.clearValueCount = 2;
    offscreenRenderPassInfo.pClearValues = clearValues;

    vkCmdBeginRenderPass(renderData.rdCommandBuffer, &offscreenRenderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

    VkViewport viewport = {0.f,
                           static_cast<float>(renderData.rdViewportTarget.size.y),
                           static_cast<float>(renderData.rdViewportTarget.size.x),
                           -static_cast<float>(renderData.rdViewportTarget.size.y),
                           0.f,
                           1.f};
    VkRect2D scissor{{0, 0},
                     {static_cast<uint32_t>(renderData.rdViewportTarget.size.x),
                      static_cast<uint32_t>(renderData.rdViewportTarget.size.y)}};

    vkCmdSetViewport(renderData.rdCommandBuffer, 0, 1, &viewport);
    vkCmdSetScissor(renderData.rdCommandBuffer, 0, 1, &scissor);
}

void Core::Renderer::VkRenderer::endOffscreenRenderPass(Core::Renderer::VkRenderData& renderData)
{
    vkCmdEndRenderPass(renderData.rdCommandBuffer);
}

void Core::Renderer::VkRenderer::beginFinalRenderPass(Core::Renderer::VkRenderData& renderData)
{
    VkClearValue clearValues[2] = {{{{0.f, 0.f, 0.f, 1.0f}}}, {1.0f, 0}};
    VkRenderPassBeginInfo renderPassBeginInfo{VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO};
    renderPassBeginInfo.renderPass = renderData.rdRenderpass;
    renderPassBeginInfo.framebuffer = renderData.rdFramebuffers[renderData.rdCurrentImageIndex];
    renderPassBeginInfo.renderArea.extent = renderData.rdVkbSwapchain.extent;
    renderPassBeginInfo.clearValueCount = 2;
    renderPassBeginInfo.pClearValues = clearValues;

    vkCmdBeginRenderPass(renderData.rdCommandBuffer, &renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);

    VkViewport viewport = {0.f,
                           static_cast<float>(renderData.rdVkbSwapchain.extent.height),
                           static_cast<float>(renderData.rdVkbSwapchain.extent.width),
                           -static_cast<float>(renderData.rdVkbSwapchain.extent.height),
                           0.f,
                           1.f};
    VkRect2D scissor = {{0, 0}, renderData.rdVkbSwapchain.extent};

    vkCmdSetViewport(renderData.rdCommandBuffer, 0, 1, &viewport);
    vkCmdSetScissor(renderData.rdCommandBuffer, 0, 1, &scissor);
}

void Core::Renderer::VkRenderer::endFinalRenderPass(Core::Renderer::VkRenderData& renderData)
{
    vkCmdEndRenderPass(renderData.rdCommandBuffer);
}

void Core::Renderer::VkRenderer::endRenderFrame(Core::Renderer::VkRenderData& renderData)
{
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

    Core::Renderer::Pipeline::cleanup(renderData, renderData.rdMeshPipeline);
    Core::Renderer::DebugSkeletonPipeline::cleanup(renderData, renderData.rdDebugSkeletonPipeline);
    Core::Renderer::Pipeline::cleanup(renderData, renderData.rdGridPipeline);
    Core::Renderer::Pipeline::cleanup(renderData, renderData.rdSkyboxPipeline);

    Core::Renderer::PipelineLayout::cleanup(renderData, renderData.rdPipelineLayout);
    Core::Renderer::MeshPipelineLayout::cleanup(renderData, renderData.rdMeshPipelineLayout);
    Core::Renderer::DebugSkeletonPipelineLayout::cleanup(renderData, renderData.rdDebugSkeletonPipelineLayout);
    Core::Renderer::PipelineLayout::cleanup(renderData, renderData.rdSkyboxPipelineLayout);

    Core::Renderer::ViewportRenderpass::cleanup(renderData);
    Core::Renderer::Renderpass::cleanup(renderData);

    Core::Renderer::UniformBuffer::cleanup(renderData, renderData.rdPerspectiveViewMatrixUBO);
    Core::Renderer::UniformBuffer::cleanup(renderData, renderData.rdCaptureUBO);
    Core::Renderer::VertexBuffer::cleanup(renderData, renderData.rdVertexBufferData);

    Core::Renderer::Texture::cleanup(renderData, renderData.rdPlaceholderTexture);

    Core::Renderer::UniformBuffer::cleanup(renderData, renderData.rdDummyBonesUBO);

    Core::Renderer::Cubemap::cleanup(renderData, renderData.rdSkyboxData);

    if (mViewportTarget)
    {
        mViewportTarget->cleanup(Core::Engine::getInstance().getRenderData());
    }

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
    auto instRet = instBuild
                       .use_default_debug_messenger()
#if defined(_DEBUG) || !defined(NDEBUG)
                       .request_validation_layers()
#endif
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
    auto& renderData = Core::Engine::getInstance().getRenderData();

    while (renderData.rdWidth == 0 || renderData.rdHeight == 0)
    {
        glfwGetFramebufferSize(renderData.rdWindow, &renderData.rdWidth, &renderData.rdHeight);
        glfwWaitEvents();
    }

    vkDeviceWaitIdle(renderData.rdVkbDevice.device);

    Core::Renderer::Framebuffer::cleanup(renderData);
    vkDestroyImageView(renderData.rdVkbDevice.device, renderData.rdDepthImageView, nullptr);
    vmaDestroyImage(renderData.rdAllocator, renderData.rdDepthImage, renderData.rdDepthImageAlloc);

    renderData.rdVkbSwapchain.destroy_image_views(renderData.rdSwapchainImageViews);

    if (mViewportTarget)
    {
        mViewportTarget->cleanup(renderData);
    }

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

    if (mViewportTarget &&
        !mViewportTarget->init(renderData, {renderData.rdViewportTarget.size.x, renderData.rdViewportTarget.size.y}))
    {
        Logger::log(1, "%s error: could not recreate viewport target\n", __FUNCTION__);
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

bool Core::Renderer::VkRenderer::createViewportRenderpass()
{
    if (!Core::Renderer::ViewportRenderpass::init(Core::Engine::getInstance().getRenderData()))
    {
        Logger::log(1, "%s error: could not init viewport renderpass\n", __FUNCTION__);
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

bool Core::Renderer::VkRenderer::createGridPipeline()
{
    const std::string vertexShaderFile = "shaders/grid.vert.spv";
    const std::string fragmentShaderFile = "shaders/grid.frag.spv";

    PipelineConfig pipelineConfig{};
    pipelineConfig.enableBlending = VK_TRUE;
    pipelineConfig.srcAlphaBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
    pipelineConfig.dstAlphaBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;

    auto& renderData = Core::Engine::getInstance().getRenderData();

    if (!Pipeline::init(renderData, renderData.rdPipelineLayout, renderData.rdGridPipeline,
                        VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST, vertexShaderFile, fragmentShaderFile, pipelineConfig))
    {
        Logger::log(1, "%s error: could not init grid shader pipeline\n", __FUNCTION__);
        return false;
    }
    return true;
}

void Core::Renderer::VkRenderer::drawGrid() const
{
    auto& renderData = Core::Engine::getInstance().getRenderData();

    vkCmdBindPipeline(renderData.rdCommandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, renderData.rdGridPipeline);

    vkCmdBindDescriptorSets(renderData.rdCommandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, renderData.rdPipelineLayout, 0,
                            1, &renderData.rdPerspectiveViewMatrixUBO.rdUBODescriptorSet, 0, nullptr);

    VkDeviceSize offset = 0;
    vkCmdBindVertexBuffers(renderData.rdCommandBuffer, 0, 1, &renderData.rdVertexBufferData.rdVertexBuffer, &offset);

    vkCmdBindPipeline(renderData.rdCommandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, renderData.rdGridPipeline);
    vkCmdDraw(renderData.rdCommandBuffer, 6, 1, 0, 0);
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

void Core::Renderer::VkRenderer::resizeViewportTarget(glm::int2 size)
{
    auto& renderData = Core::Engine::getInstance().getRenderData();

    VkDevice device = renderData.rdVkbDevice.device;

    vkDeviceWaitIdle(device);

    mViewportTarget->cleanup(renderData);

    if (!mViewportTarget->init(renderData, size))
    {
        Logger::log(1, "resizeViewportTarget: failed to recreate viewport target\n");
        return;
    }

    Logger::log(1, "%s: recreated viewport target %ix%i\n", __FUNCTION__, renderData.rdViewportTarget.size.x,
                renderData.rdViewportTarget.size.y);
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
    const std::string textureFileName = "placeholder_sampler.png";
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

bool Core::Renderer::VkRenderer::createDummyBonesTransformUBO()
{
    auto& renderData = Core::Engine::getInstance().getRenderData();

    std::vector<glm::mat4> identityMatrix = {glm::mat4(1.0f)};

    if (!UniformBuffer::init(renderData, renderData.rdDummyBonesUBO, sizeof(glm::mat4), "DummyBonesUBO"))
    {
        Logger::log(1, "%s error: could not create dummy bones UBO\n", __FUNCTION__);
        return false;
    }

    UniformBuffer::uploadData(renderData, renderData.rdDummyBonesUBO, identityMatrix);

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
    auto& renderData = Core::Engine::getInstance().getRenderData();

    std::string vertexShaderFile = "shaders/primitive.vert.spv";
    std::string fragmentShaderFile = "shaders/primitive.frag.spv";
    if (!Core::Renderer::MeshPipelineLayout::init(renderData, renderData.rdMeshPipelineLayout))
    {
        Logger::log(1, "%s error: could not init mesh pipeline layout\n", __FUNCTION__);
        return false;
    }

    VkDescriptorPoolSize poolSize{};
    poolSize.type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    poolSize.descriptorCount = MAX_MATERIALS * 5;

    VkDescriptorPoolCreateInfo poolInfo{};
    poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    poolInfo.poolSizeCount = 1;
    poolInfo.pPoolSizes = &poolSize;
    poolInfo.maxSets = MAX_MATERIALS;

    if (vkCreateDescriptorPool(renderData.rdVkbDevice.device, &poolInfo, nullptr,
                               &renderData.rdMaterialDescriptorPool) != VK_SUCCESS)
    {
        Logger::log(1, "failed to create material descriptor pool!\n");
        return false;
    }

    PipelineConfig pipelineConfig{};
    pipelineConfig.enableBlending = VK_TRUE;
    pipelineConfig.enableDepthTest = VK_TRUE;
    pipelineConfig.enableDepthWrite = VK_TRUE;
    pipelineConfig.depthCompareOp = VK_COMPARE_OP_LESS;

    if (!Pipeline::init(renderData, renderData.rdMeshPipelineLayout, renderData.rdMeshPipeline,
                        VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST, vertexShaderFile, fragmentShaderFile, pipelineConfig))
    {
        Logger::log(1, "%s error: could not init mesh pipeline\n", __FUNCTION__);
        return false;
    }

    const std::string modelFileName = "assets/damaged_helmet/DamagedHelmet.gltf";

    Core::Utils::MeshData primitiveMeshData = Core::Utils::loadMeshFromFile(modelFileName, renderData);

    createDebugSkeletonPipelineLayout();
    createDebugSkeletonPipeline();

    auto testMesh = std::make_shared<Mesh>("TestMesh", primitiveMeshData.skeleton);
    testMesh->setupAnimations(primitiveMeshData.animations);
    testMesh->initDebugSkeleton(renderData);
    testMesh->getTransform().position = {1, 0, 1};
    testMesh->setMeshFilePath(modelFileName);

    Core::Engine::getInstance().getSystem<Scene::Scene>()->addObject(testMesh);

    for (auto& primitive : primitiveMeshData.primitives)
    {
        testMesh->addPrimitive(primitive.vertices, primitive.indices, primitive.textures, renderData,
                               primitive.material, primitive.bones, primitive.materialDescriptorSet);
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

bool Core::Renderer::VkRenderer::createHDRToCubemapPipelineLayout()
{
    auto& renderData = Core::Engine::getInstance().getRenderData();

    std::vector<VkDescriptorSetLayout> layouts = {
        renderData.rdCaptureUBO.rdUBODescriptorLayout,
        renderData.rdHDRTexture.descriptorSetLayout
    };

    VkPushConstantRange pushConstantRange{};
    pushConstantRange.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
    pushConstantRange.offset = 0;
    pushConstantRange.size = sizeof(int);

    // TODO
    // replace with PipelineLayout::init and PipelineLayoutConfig
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

    return true;
}

bool Core::Renderer::VkRenderer::createHDRToCubemapPipeline()
{
    const std::string vertexShaderFile = "shaders/equirectangular_to_cubemap.vert.spv";
    const std::string fragmentShaderFile = "shaders/equirectangular_to_cubemap.frag.spv";

    auto& renderData = Core::Engine::getInstance().getRenderData();

    PipelineConfig hdrToCubemapConfig{};
    hdrToCubemapConfig.useVertexInput = VK_FALSE;
    hdrToCubemapConfig.enableDepthTest = VK_FALSE;
    hdrToCubemapConfig.enableDepthWrite = VK_FALSE;
    hdrToCubemapConfig.enableBlending = VK_FALSE;
    hdrToCubemapConfig.cullMode = VK_CULL_MODE_NONE;
    hdrToCubemapConfig.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
    hdrToCubemapConfig.depthCompareOp = VK_COMPARE_OP_ALWAYS;

    if (!Pipeline::init(renderData, renderData.rdHDRToCubemapPipelineLayout, renderData.rdHDRToCubemapPipeline,
                        VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST, vertexShaderFile, fragmentShaderFile, hdrToCubemapConfig,
                        renderData.rdHDRToCubemapRenderpass))
    {
        Logger::log(1, "%s error: could not init HDR to Cubemap pipeline\n", __FUNCTION__);
        return false;
    }

    return true;
}

bool Core::Renderer::VkRenderer::createSkyboxPipelineLayout()
{
    auto& renderData = Core::Engine::getInstance().getRenderData();

    std::vector<VkDescriptorSetLayout> layouts = {renderData.rdPerspectiveViewMatrixUBO.rdUBODescriptorLayout};

    if (renderData.rdSkyboxData.descriptorSetLayout != VK_NULL_HANDLE)
    {
        layouts.push_back(renderData.rdSkyboxData.descriptorSetLayout);
    }

    VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
    pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    pipelineLayoutInfo.setLayoutCount = static_cast<uint32_t>(layouts.size());
    pipelineLayoutInfo.pSetLayouts = layouts.data();

    if (vkCreatePipelineLayout(renderData.rdVkbDevice.device, &pipelineLayoutInfo, nullptr,
                               &renderData.rdSkyboxPipelineLayout) != VK_SUCCESS)
    {
        Logger::log(1, "%s error: could not create cubemap pipeline layout", __FUNCTION__);
        return false;
    }

    return true;
}

bool Core::Renderer::VkRenderer::createSkyboxPipeline()
{
    const std::string vertexShaderFile = "shaders/skybox.vert.spv";
    const std::string fragmentShaderFile = "shaders/skybox.frag.spv";

    auto& renderData = Core::Engine::getInstance().getRenderData();

    if (!Pipeline::init(renderData, renderData.rdSkyboxPipelineLayout, renderData.rdSkyboxPipeline,
                        VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST, vertexShaderFile, fragmentShaderFile))
    {
        Logger::log(1, "%s error: could not init cubemap pipeline\n", __FUNCTION__);
        return false;
    }

    return true;
}

bool Core::Renderer::VkRenderer::createIrradiancePipelineLayout()
{
    auto& renderData = Core::Engine::getInstance().getRenderData();

    std::vector layouts = {
        renderData.rdCaptureUBO.rdUBODescriptorLayout,
        renderData.rdSkyboxData.descriptorSetLayout 
    };

    VkPushConstantRange pushConstantRange{};
    pushConstantRange.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
    pushConstantRange.offset = 0;
    pushConstantRange.size = sizeof(int);

    // TODO
    // replace with PipelineLayout::init and PipelineLayoutConfig
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

    return true;
}

bool Core::Renderer::VkRenderer::createIrradiancePipeline()
{
    const std::string vertexShaderFile = "shaders/equirectangular_to_cubemap.vert.spv";
    const std::string fragmentShaderFile = "shaders/irradiance_convolution.frag.spv";

    auto& renderData = Core::Engine::getInstance().getRenderData();

    PipelineConfig irradiancePipelineConfig{};
    irradiancePipelineConfig.useVertexInput = VK_FALSE;
    irradiancePipelineConfig.enableDepthTest = VK_FALSE;
    irradiancePipelineConfig.enableDepthWrite = VK_FALSE;
    irradiancePipelineConfig.enableBlending = VK_FALSE;
    irradiancePipelineConfig.cullMode = VK_CULL_MODE_NONE;
    irradiancePipelineConfig.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
    irradiancePipelineConfig.depthCompareOp = VK_COMPARE_OP_ALWAYS;

    if (!Pipeline::init(renderData, renderData.rdIrradiancePipelineLayout, renderData.rdIrradiancePipeline,
                        VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST, vertexShaderFile, fragmentShaderFile, irradiancePipelineConfig,
                        renderData.rdIBLRenderpass))
    {
        Logger::log(1, "%s error: could not init irradiance pipeline\n", __FUNCTION__);
        return false;
    }

    return true;
}

bool Core::Renderer::VkRenderer::createPrefilterPipelineLayout()
{
    auto& renderData = Core::Engine::getInstance().getRenderData();

    std::vector layouts = {
        renderData.rdCaptureUBO.rdUBODescriptorLayout,
        renderData.rdSkyboxData.descriptorSetLayout
    };

    VkPushConstantRange pushConstantRange{};
    pushConstantRange.stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;
    pushConstantRange.offset = 0;
    pushConstantRange.size = sizeof(uint32_t) + sizeof(float);

    // TODO
    // replace with PipelineLayout::init and PipelineLayoutConfig
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

    return true;
}

bool Core::Renderer::VkRenderer::createPrefilterPipeline()
{
    const std::string vertexShaderFile = "shaders/prefilter.vert.spv";
    const std::string fragmentShaderFile = "shaders/prefilter.frag.spv";

    auto& renderData = Core::Engine::getInstance().getRenderData();

    PipelineConfig prefilterPipelineConfig{};
    prefilterPipelineConfig.useVertexInput = VK_FALSE;
    prefilterPipelineConfig.enableDepthTest = VK_FALSE;
    prefilterPipelineConfig.enableDepthWrite = VK_FALSE;
    prefilterPipelineConfig.enableBlending = VK_FALSE;
    prefilterPipelineConfig.cullMode = VK_CULL_MODE_NONE;
    prefilterPipelineConfig.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
    prefilterPipelineConfig.depthCompareOp = VK_COMPARE_OP_ALWAYS;

    if (!Pipeline::init(renderData, renderData.rdPrefilterPipelineLayout, renderData.rdPrefilterPipeline,
                        VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST, vertexShaderFile, fragmentShaderFile, prefilterPipelineConfig,
                        renderData.rdIBLRenderpass))
    {
        Logger::log(1, "%s error: could not init irradiance pipeline\n", __FUNCTION__);
        return false;
    }

    return true;
}

bool Core::Renderer::VkRenderer::createBRDFLUTPipelineLayout()
{
    auto& renderData = Core::Engine::getInstance().getRenderData();

    // TODO
    // replace with PipelineLayout::init and PipelineLayoutConfig
    VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
    pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;

    if (vkCreatePipelineLayout(renderData.rdVkbDevice.device, &pipelineLayoutInfo, nullptr,
                               &renderData.rdBRDFLUTPipelineLayout) != VK_SUCCESS)
    {
        Logger::log(1, "%s error: could not create BRDF LUT pipeline layout", __FUNCTION__);
        return false;
    }

    return true;
}

bool Core::Renderer::VkRenderer::createBRDFLUTPipeline()
{
    const std::string vertexShaderFile = "shaders/brdf_lut.vert.spv";
    const std::string fragmentShaderFile = "shaders/brdf_lut.frag.spv";

    auto& renderData = Core::Engine::getInstance().getRenderData();

    PipelineConfig brdflutPipelineConfig{};
    brdflutPipelineConfig.useVertexInput = VK_FALSE;
    brdflutPipelineConfig.enableDepthTest = VK_FALSE;
    brdflutPipelineConfig.enableDepthWrite = VK_FALSE;
    brdflutPipelineConfig.enableBlending = VK_FALSE;
    brdflutPipelineConfig.cullMode = VK_CULL_MODE_NONE;
    brdflutPipelineConfig.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
    brdflutPipelineConfig.depthCompareOp = VK_COMPARE_OP_ALWAYS;

    if (!Pipeline::init(renderData, renderData.rdBRDFLUTPipelineLayout, renderData.rdBRDFLUTPipeline,
                        VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST, vertexShaderFile, fragmentShaderFile, brdflutPipelineConfig,
                        renderData.rdIBLRenderpass))
    {
        Logger::log(1, "%s error: could not init BRDF LUT pipeline\n", __FUNCTION__);
        return false;
    }

    return true;
}

bool Core::Renderer::VkRenderer::loadSkybox()
{
    auto& renderData = Core::Engine::getInstance().getRenderData();

    if (!Cubemap::loadHDRTexture(renderData, renderData.rdHDRTexture, "assets/textures/hdr/skybox.hdr"))
    {
        Logger::log(1, "%s error: could not load HDR texture", __FUNCTION__);
        return false;
    }

    CaptureInfo captureInfo{};
    captureInfo.projection = glm::perspective(glm::radians(90.0f), 1.0f, 0.1f, 10.0f);
    captureInfo.views[0] = glm::lookAt(glm::vec3(0), glm::vec3(1,0,0), glm::vec3(0,-1,0));
    captureInfo.views[1] = glm::lookAt(glm::vec3(0), glm::vec3(-1,0,0), glm::vec3(0,-1,0));
    captureInfo.views[2] = glm::lookAt(glm::vec3(0), glm::vec3(0,1,0), glm::vec3(0,0,1));
    captureInfo.views[3] = glm::lookAt(glm::vec3(0), glm::vec3(0,-1,0), glm::vec3(0,0,-1));
    captureInfo.views[4] = glm::lookAt(glm::vec3(0), glm::vec3(0,0,1), glm::vec3(0,-1,0));
    captureInfo.views[5] = glm::lookAt(glm::vec3(0), glm::vec3(0,0,-1), glm::vec3(0,-1,0));

    UniformBuffer::init(renderData, renderData.rdCaptureUBO, sizeof(CaptureInfo), "CaptureInfo");
    UniformBuffer::uploadData(renderData, renderData.rdCaptureUBO, captureInfo);

    if (!HDRToCubemapRenderpass::init(Core::Engine::getInstance().getRenderData(), renderData.rdHDRToCubemapRenderpass))
    {
        Logger::log(1, "%s error: could not init HDR to Cubemap renderpass\n", __FUNCTION__);
        return false;
    }

    // TODO
    // move from here lol
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

        vkCreateDescriptorSetLayout(renderData.rdVkbDevice.device, &layoutInfo, nullptr,
                                    &renderData.rdHDRTexture.descriptorSetLayout);

        VkDescriptorPoolSize poolSize{};
        poolSize.type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        poolSize.descriptorCount = 1;

        VkDescriptorPoolCreateInfo poolInfo{};
        poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
        poolInfo.poolSizeCount = 1;
        poolInfo.pPoolSizes = &poolSize;
        poolInfo.maxSets = 1;

        vkCreateDescriptorPool(renderData.rdVkbDevice.device, &poolInfo, nullptr,
                               &renderData.rdHDRTexture.descriptorPool);

        VkDescriptorSetAllocateInfo allocInfo{};
        allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
        allocInfo.descriptorPool = renderData.rdHDRTexture.descriptorPool;
        allocInfo.descriptorSetCount = 1;
        allocInfo.pSetLayouts = &renderData.rdHDRTexture.descriptorSetLayout;

        vkAllocateDescriptorSets(renderData.rdVkbDevice.device, &allocInfo,
                                 &renderData.rdHDRTexture.descriptorSet);

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

        vkUpdateDescriptorSets(renderData.rdVkbDevice.device, 1, &write, 0, nullptr);
    }
    //

    if (!createHDRToCubemapPipelineLayout())
    {
        Logger::log(1, "%s error: could not create HDR to Cubemap pipeline layout\n", __FUNCTION__);
        return false;
    }

    if (!createHDRToCubemapPipeline())
    {
        Logger::log(1, "%s error: could not create HDR to Cubemap pipeline\n", __FUNCTION__);
        return false;
    }

    if (!Cubemap::convertHDRToCubemap(renderData, renderData.rdHDRTexture, renderData.rdSkyboxData))
    {
        Logger::log(1, "%s error: could not convert HDR to Cubemap", __FUNCTION__);
        return false;
    }

    if (!createSkyboxPipelineLayout())
    {
        Logger::log(1, "%s error: could not create skybox pipeline layout\n", __FUNCTION__);
        return false;
    }

    if (!createSkyboxPipeline())
    {
        Logger::log(1, "%s error: could not create skybox pipeline\n", __FUNCTION__);
        return false;
    }

    if (!HDRToCubemapRenderpass::init(Core::Engine::getInstance().getRenderData(), renderData.rdIBLRenderpass))
    {
        Logger::log(1, "%s error: could not init irradiance renderpass\n", __FUNCTION__);
        return false;
    }

    if (!createIrradiancePipelineLayout())
    {
        Logger::log(1, "%s error: could not creat irradiance pipeline layout\n", __FUNCTION__);
        return false;
    }

    if (!createIrradiancePipeline())
    {
        Logger::log(1, "%s error: could not create irradiance pipeline\n", __FUNCTION__);
        return false;
    }

    if (!Cubemap::convertCubemapToIrradiance(renderData, renderData.rdSkyboxData, renderData.rdIrradianceMap))
    {
        Logger::log(1, "%s error: could not convert cubemap to irradiance", __FUNCTION__);
        return false;
    }

    if (!createPrefilterPipelineLayout())
    {
        Logger::log(1, "%s error: could not creat prefilter pipeline layout\n", __FUNCTION__);
        return false;
    }

    if (!createPrefilterPipeline())
    {
        Logger::log(1, "%s error: could not create prefilter pipeline\n", __FUNCTION__);
        return false;
    }

    if (!Cubemap::convertCubemapToPrefilteredMap(renderData, renderData.rdSkyboxData, renderData.rdPrefilterMap))
    {
        Logger::log(1, "%s error: could not convert cubemap to irradiance", __FUNCTION__);
        return false;
    }

    if (!createBRDFLUTPipelineLayout())
    {
        Logger::log(1, "%s error: could not creat BRDF LUT pipeline layout\n", __FUNCTION__);
        return false;
    }

    if (!createBRDFLUTPipeline())
    {
        Logger::log(1, "%s error: could not create BRDF LUT pipeline\n", __FUNCTION__);
        return false;
    }

    if (!Cubemap::generateBRDFLUT(renderData, renderData.rdBRDFLUT))
    {
        Logger::log(1, "%s error: could not generate BRDF LUT", __FUNCTION__);
        return false;
    }

    Logger::log(1, "%s: cubemap loaded successfully\n", __FUNCTION__);
    return true;
}

void Core::Renderer::VkRenderer::drawSkybox() const
{
    auto& renderData = Core::Engine::getInstance().getRenderData();

    if (renderData.rdSkyboxPipeline == VK_NULL_HANDLE || renderData.rdSkyboxData.descriptorSet == VK_NULL_HANDLE)
    {
        return;
    }

    vkCmdBindPipeline(renderData.rdCommandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, renderData.rdSkyboxPipeline);

    std::vector<VkDescriptorSet> descriptorSets = {renderData.rdPerspectiveViewMatrixUBO.rdUBODescriptorSet,
                                                   renderData.rdSkyboxData.descriptorSet};

    vkCmdBindDescriptorSets(renderData.rdCommandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS,
                            renderData.rdSkyboxPipelineLayout, 0, static_cast<uint32_t>(descriptorSets.size()),
                            descriptorSets.data(), 0, nullptr);

    VkDeviceSize offset = 0;
    vkCmdBindVertexBuffers(renderData.rdCommandBuffer, 0, 1, &renderData.rdVertexBufferData.rdVertexBuffer, &offset);

    vkCmdDraw(renderData.rdCommandBuffer, 36, 1, 0, 0);
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
