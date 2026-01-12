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

#include "core/components/TransformComponent.h"
#include "core/vk-renderer/pipelines/DebugSkeletonPipeline.h"
#include "core/engine/Engine.h"
#include "core/vk-renderer/viewport/ViewportRenderpass.h"
#include "core/vk-renderer/viewport/ViewportTarget.h"
#include "cubemap_generator/HDRToCubemapRenderpass.h"
#include "ibl_generator/IBLGenerator.h"

bool Core::Renderer::VkRenderer::init(const unsigned int width, const unsigned int height)
{
    if (!Engine::getInstance().getRenderData().rdWindow)
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

    if (!createPrimitivePipeline())
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

    Engine::getInstance().getRenderData().rdWidth = static_cast<int>(width);
    Engine::getInstance().getRenderData().rdHeight = static_cast<int>(height);

    Logger::log(1, "%s: Vulkan Core::Renderer initialized to %ix%i\n", __FUNCTION__, width, height);
    return true;
}

Core::Renderer::VkRenderer::VkRenderer(GLFWwindow* inWindow)
{
    Engine::getInstance().getRenderData().rdWindow = inWindow;
}

void Core::Renderer::VkRenderer::handleWindowResizeEvents(int width, int height)
{
    VkRenderData& renderData = Engine::getInstance().getRenderData();

    vkDeviceWaitIdle(renderData.rdVkbDevice.device);

    renderData.rdWidth = width;
    renderData.rdHeight = height;

    recreateSwapchain();

    vkResetCommandBuffer(renderData.rdCommandBuffer, 0);

    Logger::log(1, "%s: resized window to %ix%i\n", __FUNCTION__, width, height);
}

void Core::Renderer::VkRenderer::onEvent(const Event& event)
{
    VkRenderData& renderData = Engine::getInstance().getRenderData();

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

void Core::Renderer::VkRenderer::beginUploadFrame(VkRenderData& renderData)
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

    updateGlobalSceneData();
}

void Core::Renderer::VkRenderer::endUploadFrame(VkRenderData& renderData)
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

void Core::Renderer::VkRenderer::beginRenderFrame(VkRenderData& renderData)
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

void Core::Renderer::VkRenderer::beginOffscreenRenderPass(VkRenderData& renderData)
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

void Core::Renderer::VkRenderer::endOffscreenRenderPass(VkRenderData& renderData)
{
    vkCmdEndRenderPass(renderData.rdCommandBuffer);
}

void Core::Renderer::VkRenderer::beginFinalRenderPass(VkRenderData& renderData)
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

void Core::Renderer::VkRenderer::endFinalRenderPass(VkRenderData& renderData)
{
    vkCmdEndRenderPass(renderData.rdCommandBuffer);
}

void Core::Renderer::VkRenderer::endRenderFrame(VkRenderData& renderData)
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
    SyncObjects::cleanup(renderData);

    CommandBuffer::cleanup(renderData, Engine::getInstance().getRenderData().rdCommandBuffer);
    CommandPool::cleanup(renderData);

    Framebuffer::cleanup(renderData);

    Pipeline::cleanup(renderData, renderData.rdMeshPipeline);
    DebugSkeletonPipeline::cleanup(renderData, renderData.rdDebugSkeletonPipeline);
    Pipeline::cleanup(renderData, renderData.rdGridPipeline);
    Pipeline::cleanup(renderData, renderData.rdSkyboxPipeline);
    Pipeline::cleanup(renderData, renderData.rdHDRToCubemapPipeline);
    Pipeline::cleanup(renderData, renderData.rdIrradiancePipeline);
    Pipeline::cleanup(renderData, renderData.rdPrefilterPipeline);
    Pipeline::cleanup(renderData, renderData.rdBRDFLUTPipeline);

    MeshPipelineLayout::cleanup(renderData, renderData.rdMeshPipelineLayout);
    PipelineLayout::cleanup(renderData, renderData.rdPipelineLayout);
    PipelineLayout::cleanup(renderData, renderData.rdDebugSkeletonPipelineLayout);
    PipelineLayout::cleanup(renderData, renderData.rdSkyboxPipelineLayout);
    PipelineLayout::cleanup(renderData, renderData.rdHDRToCubemapPipelineLayout);
    PipelineLayout::cleanup(renderData, renderData.rdIrradiancePipelineLayout);
    PipelineLayout::cleanup(renderData, renderData.rdPrefilterPipelineLayout);
    PipelineLayout::cleanup(renderData, renderData.rdBRDFLUTPipelineLayout);

    ViewportRenderpass::cleanup(renderData);
    Renderpass::cleanup(renderData);
    HDRToCubemapRenderpass::cleanup(renderData, renderData.rdIBLRenderpass);
    HDRToCubemapRenderpass::cleanup(renderData, renderData.rdHDRToCubemapRenderpass);

    UniformBuffer::cleanup(renderData, renderData.rdGlobalSceneUBO);
    UniformBuffer::cleanup(renderData, renderData.rdCaptureUBO);
    VertexBuffer::cleanup(renderData, renderData.rdVertexBufferData);

    Texture::cleanup(renderData, renderData.rdPlaceholderTexture);
    Texture::cleanup(renderData, renderData.rdBRDFLUT);
    Texture::cleanup(renderData, renderData.rdHDRTexture);

    UniformBuffer::cleanup(renderData, renderData.rdDummyBonesUBO);

    IBLGenerator::cleanup(renderData, renderData.rdPrefilterMap);
    IBLGenerator::cleanup(renderData, renderData.rdIrradianceMap);
    IBLGenerator::cleanup(renderData, renderData.rdSkyboxData);

    if (mViewportTarget)
    {
        mViewportTarget->cleanup(Engine::getInstance().getRenderData());
    }

    vkDestroyImageView(renderData.rdVkbDevice.device, renderData.rdDepthImageView, nullptr);
    vmaDestroyImage(renderData.rdAllocator, renderData.rdDepthImage,
                    Engine::getInstance().getRenderData().rdDepthImageAlloc);

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
    Engine::getInstance().getRenderData().rdVkbInstance = instRet.value();

    VkResult result = VK_ERROR_UNKNOWN;
    result = glfwCreateWindowSurface(Engine::getInstance().getRenderData().rdVkbInstance,
                                     Engine::getInstance().getRenderData().rdWindow, nullptr, &mSurface);
    if (result != VK_SUCCESS)
    {
        Logger::log(1, "%s error: Could not create Vulkan surface\n", __FUNCTION__);
        return false;
    }

    /* just get the first available device */
    vkb::PhysicalDeviceSelector physicalDevSel{Engine::getInstance().getRenderData().rdVkbInstance};
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

    Engine::getInstance().getRenderData().rdVkbPhysicalDevice = secondPhysicalDevSelRet.value();

    Logger::log(1, "%s: found physical device '%s'\n", __FUNCTION__,
                Engine::getInstance().getRenderData().rdVkbPhysicalDevice.name.c_str());

    mMinUniformBufferOffsetAlignment = Engine::getInstance()
                                           .getRenderData()
                                           .rdVkbPhysicalDevice.properties.limits.minUniformBufferOffsetAlignment;
    Logger::log(1, "%s: the physical device as a minimal uniform buffer offset of %i bytes\n", __FUNCTION__,
                mMinUniformBufferOffsetAlignment);

    vkb::DeviceBuilder devBuilder{Engine::getInstance().getRenderData().rdVkbPhysicalDevice};
    auto devBuilderRet = devBuilder.build();
    if (!devBuilderRet)
    {
        Logger::log(1, "%s error: could not get devices\n", __FUNCTION__);
        return false;
    }
    Engine::getInstance().getRenderData().rdVkbDevice = devBuilderRet.value();

    return true;
}

bool Core::Renderer::VkRenderer::getQueue()
{
    auto graphQueueRet = Engine::getInstance().getRenderData().rdVkbDevice.get_queue(vkb::QueueType::graphics);
    if (!graphQueueRet.has_value())
    {
        Logger::log(1, "%s error: could not get graphics queue\n", __FUNCTION__);
        return false;
    }
    Engine::getInstance().getRenderData().rdGraphicsQueue = graphQueueRet.value();

    auto presentQueueRet = Engine::getInstance().getRenderData().rdVkbDevice.get_queue(vkb::QueueType::present);
    if (!presentQueueRet.has_value())
    {
        Logger::log(1, "%s error: could not get present queue\n", __FUNCTION__);
        return false;
    }
    Engine::getInstance().getRenderData().rdPresentQueue = presentQueueRet.value();

    return true;
}

bool Core::Renderer::VkRenderer::createSwapchain()
{
    vkb::SwapchainBuilder swapChainBuild{Engine::getInstance().getRenderData().rdVkbDevice};

    glfwGetFramebufferSize(Engine::getInstance().getRenderData().rdWindow,
                           &Engine::getInstance().getRenderData().rdWidth,
                           &Engine::getInstance().getRenderData().rdHeight);

    auto swapChainBuildRet =
        swapChainBuild.set_old_swapchain(Engine::getInstance().getRenderData().rdVkbSwapchain)
            .set_desired_present_mode(VK_PRESENT_MODE_FIFO_KHR)
            .set_desired_extent(Engine::getInstance().getRenderData().rdWidth,
                                Engine::getInstance().getRenderData().rdHeight)
            .build();
    if (!swapChainBuildRet)
    {
        Logger::log(1, "%s error: could not init swapchain\n", __FUNCTION__);
        return false;
    }

    vkb::destroy_swapchain(Engine::getInstance().getRenderData().rdVkbSwapchain);
    Engine::getInstance().getRenderData().rdVkbSwapchain = swapChainBuildRet.value();

    return true;
}

bool Core::Renderer::VkRenderer::createDepthBuffer()
{
    const VkExtent3D depthImageExtent = {Engine::getInstance().getRenderData().rdVkbSwapchain.extent.width,
                                         Engine::getInstance().getRenderData().rdVkbSwapchain.extent.height, 1};

    Engine::getInstance().getRenderData().rdDepthFormat = VK_FORMAT_D32_SFLOAT;

    VkImageCreateInfo depthImageInfo{};
    depthImageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    depthImageInfo.imageType = VK_IMAGE_TYPE_2D;
    depthImageInfo.format = Engine::getInstance().getRenderData().rdDepthFormat;
    depthImageInfo.extent = depthImageExtent;
    depthImageInfo.mipLevels = 1;
    depthImageInfo.arrayLayers = 1;
    depthImageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
    depthImageInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
    depthImageInfo.usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;

    VmaAllocationCreateInfo depthAllocInfo{};
    depthAllocInfo.usage = VMA_MEMORY_USAGE_GPU_ONLY;
    depthAllocInfo.requiredFlags = static_cast<VkMemoryPropertyFlags>(VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

    if (vmaCreateImage(Engine::getInstance().getRenderData().rdAllocator, &depthImageInfo, &depthAllocInfo,
                       &Engine::getInstance().getRenderData().rdDepthImage,
                       &Engine::getInstance().getRenderData().rdDepthImageAlloc, nullptr) != VK_SUCCESS)
    {
        Logger::log(1, "%s error: could not allocate depth buffer memory\n", __FUNCTION__);
        return false;
    }

    VkImageViewCreateInfo depthImageViewInfo{};
    depthImageViewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    depthImageViewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
    depthImageViewInfo.image = Engine::getInstance().getRenderData().rdDepthImage;
    depthImageViewInfo.format = Engine::getInstance().getRenderData().rdDepthFormat;
    depthImageViewInfo.subresourceRange.baseMipLevel = 0;
    depthImageViewInfo.subresourceRange.levelCount = 1;
    depthImageViewInfo.subresourceRange.baseArrayLayer = 0;
    depthImageViewInfo.subresourceRange.layerCount = 1;
    depthImageViewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;

    if (vkCreateImageView(Engine::getInstance().getRenderData().rdVkbDevice.device, &depthImageViewInfo, nullptr,
                          &Engine::getInstance().getRenderData().rdDepthImageView) != VK_SUCCESS)
    {
        Logger::log(1, "%s error: could not create depth buffer image view\n", __FUNCTION__);
        return false;
    }
    return true;
}

bool Core::Renderer::VkRenderer::recreateSwapchain()
{
    auto& renderData = Engine::getInstance().getRenderData();

    while (renderData.rdWidth == 0 || renderData.rdHeight == 0)
    {
        glfwGetFramebufferSize(renderData.rdWindow, &renderData.rdWidth, &renderData.rdHeight);
        glfwWaitEvents();
    }

    vkDeviceWaitIdle(renderData.rdVkbDevice.device);

    Framebuffer::cleanup(renderData);
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

bool Core::Renderer::VkRenderer::createVBO()
{
    if (!VertexBuffer::init(Engine::getInstance().getRenderData(),
                                            Engine::getInstance().getRenderData().rdVertexBufferData,
                                            VertexBufferSize, "MainVBO"))
    {
        Logger::log(1, "%s error: could not create VBO\n", __FUNCTION__);
        return false;
    }
    return true;
}

bool Core::Renderer::VkRenderer::createRenderPass()
{
    if (!Renderpass::init(Engine::getInstance().getRenderData()))
    {
        Logger::log(1, "%s error: could not init renderpass\n", __FUNCTION__);
        return false;
    }
    return true;
}

bool Core::Renderer::VkRenderer::createViewportRenderpass()
{
    if (!ViewportRenderpass::init(Engine::getInstance().getRenderData()))
    {
        Logger::log(1, "%s error: could not init viewport renderpass\n", __FUNCTION__);
        return false;
    }
    return true;
}

bool Core::Renderer::VkRenderer::createPipelineLayout()
{
    auto& renderData = Engine::getInstance().getRenderData();

    auto pipelineLayoutConfig = PipelineLayoutConfig{};
    pipelineLayoutConfig.setLayouts = {renderData.rdGlobalSceneDescriptorLayout};

    if (!PipelineLayout::init(renderData, renderData.rdPipelineLayout, pipelineLayoutConfig))
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

    auto& renderData = Engine::getInstance().getRenderData();

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
    auto& renderData = Engine::getInstance().getRenderData();

    vkCmdBindPipeline(renderData.rdCommandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, renderData.rdGridPipeline);

    vkCmdBindDescriptorSets(renderData.rdCommandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, renderData.rdPipelineLayout, 0,
                            1, &renderData.rdGlobalSceneUBO.rdUBODescriptorSet, 0, nullptr);

    VkDeviceSize offset = 0;
    vkCmdBindVertexBuffers(renderData.rdCommandBuffer, 0, 1, &renderData.rdVertexBufferData.rdVertexBuffer, &offset);

    vkCmdBindPipeline(renderData.rdCommandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, renderData.rdGridPipeline);
    vkCmdDraw(renderData.rdCommandBuffer, 6, 1, 0, 0);
}

bool Core::Renderer::VkRenderer::createFramebuffer()
{
    if (!Framebuffer::init(Engine::getInstance().getRenderData()))
    {
        Logger::log(1, "%s error: could not init framebuffer\n", __FUNCTION__);
        return false;
    }
    return true;
}

void Core::Renderer::VkRenderer::resizeViewportTarget(glm::int2 size)
{
    auto& renderData = Engine::getInstance().getRenderData();

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
    if (!CommandPool::init(Engine::getInstance().getRenderData()))
    {
        Logger::log(1, "%s error: could not create command pool\n", __FUNCTION__);
        return false;
    }
    return true;
}

bool Core::Renderer::VkRenderer::createCommandBuffer()
{
    if (!CommandBuffer::init(Engine::getInstance().getRenderData(),
                                             Engine::getInstance().getRenderData().rdCommandBuffer))
    {
        Logger::log(1, "%s error: could not create command buffers\n", __FUNCTION__);
        return false;
    }
    return true;
}

bool Core::Renderer::VkRenderer::createSyncObjects()
{
    if (!SyncObjects::init(Engine::getInstance().getRenderData()))
    {
        Logger::log(1, "%s error: could not create sync objects\n", __FUNCTION__);
        return false;
    }

    return true;
}

bool Core::Renderer::VkRenderer::loadPlaceholderTexture()
{
    const std::string textureFileName = "placeholder_sampler.png";
    std::future<bool> textureLoadFuture = Texture::loadTexture(
        Engine::getInstance().getRenderData(), Engine::getInstance().getRenderData().rdPlaceholderTexture,
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
    auto& renderData = Engine::getInstance().getRenderData();

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
    allocatorInfo.physicalDevice = Engine::getInstance().getRenderData().rdVkbPhysicalDevice.physical_device;
    allocatorInfo.device = Engine::getInstance().getRenderData().rdVkbDevice.device;
    allocatorInfo.instance = Engine::getInstance().getRenderData().rdVkbInstance.instance;
    if (vmaCreateAllocator(&allocatorInfo, &Engine::getInstance().getRenderData().rdAllocator) != VK_SUCCESS)
    {
        Logger::log(1, "%s error: could not init VMA\n", __FUNCTION__);
        return false;
    }

    return true;
}

bool Core::Renderer::VkRenderer::loadMeshWithAssimp()
{
    auto& renderData = Engine::getInstance().getRenderData();

    VkDescriptorPoolSize poolSize{};
    poolSize.type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    poolSize.descriptorCount = maxNumberOfMaterials * 5;

    VkDescriptorPoolCreateInfo poolInfo{};
    poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    poolInfo.poolSizeCount = 1;
    poolInfo.pPoolSizes = &poolSize;
    poolInfo.maxSets = maxNumberOfMaterials;

    if (vkCreateDescriptorPool(renderData.rdVkbDevice.device, &poolInfo, nullptr,
                               &renderData.rdMaterialDescriptorPool) != VK_SUCCESS)
    {
        Logger::log(1, "failed to create material descriptor pool!\n");
        return false;
    }

    const std::string modelFileName = "assets/damaged_helmet/DamagedHelmet.gltf";

    Utils::MeshData primitiveMeshData = Utils::loadMeshFromFile(modelFileName, renderData);

    createDebugSkeletonPipelineLayout();
    createDebugSkeletonPipeline();

    auto testObject = std::make_shared<Scene::SceneObject>("TestObject");

    auto transformComponent = testObject->addComponent<Component::TransformComponent>();
    transformComponent->transform.setPosition({0, 0, 0});

    auto meshComponent = testObject->addComponent<Component::MeshComponent>(primitiveMeshData.skeleton);
    for (auto& primitive : primitiveMeshData.primitives)
    {
        meshComponent->addPrimitive(primitive.vertices, primitive.indices, primitive.textures, renderData,
                               primitive.material, primitive.bones, primitive.materialDescriptorSet);
    }
    meshComponent->setupAnimations(primitiveMeshData.animations);
    meshComponent->initDebugSkeleton(renderData);
    meshComponent->setMeshFilePath(modelFileName);

    Engine::getInstance().getSystem<Scene::Scene>()->addObject(testObject);

    initPrimitiveGlobalSceneDescriptorSet();

    return true;
}

void Core::Renderer::VkRenderer::initCaptureResources()
{
    auto& renderData = Engine::getInstance().getRenderData();

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
}

void Core::Renderer::VkRenderer::initPrimitiveGlobalSceneDescriptorSet()
{
    auto& renderData = Engine::getInstance().getRenderData();

    std::vector<VkDescriptorPoolSize> extraSizes = {
        {VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 3}
    };

    UniformBuffer::init(renderData, renderData.rdGlobalSceneUBO, sizeof(GlobalSceneData),
        "GlobalScene with IBL", renderData.rdGlobalSceneDescriptorLayout, extraSizes);

    updateGlobalSceneDescriptorWrite();
}

void Core::Renderer::VkRenderer::updateGlobalSceneDescriptorWrite()
{
    auto& renderData = Engine::getInstance().getRenderData();

    VkDescriptorSet targetSet = renderData.rdGlobalSceneUBO.rdUBODescriptorSet;

    VkDescriptorImageInfo irradianceInfo{};
    irradianceInfo.sampler = renderData.rdIrradianceMap.sampler;
    irradianceInfo.imageView = renderData.rdIrradianceMap.imageView;
    irradianceInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

    VkDescriptorImageInfo prefilterInfo{};
    prefilterInfo.sampler = renderData.rdPrefilterMap.sampler;
    prefilterInfo.imageView = renderData.rdPrefilterMap.imageView;
    prefilterInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

    VkDescriptorImageInfo brdfInfo{};
    brdfInfo.sampler = renderData.rdBRDFLUT.sampler;
    brdfInfo.imageView = renderData.rdBRDFLUT.imageView;
    brdfInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

    std::vector<VkWriteDescriptorSet> descriptorWrites;

    VkWriteDescriptorSet irradianceWrite{};
    irradianceWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    irradianceWrite.dstSet = targetSet;
    irradianceWrite.dstBinding = 1;
    irradianceWrite.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    irradianceWrite.descriptorCount = 1;
    irradianceWrite.pImageInfo = &irradianceInfo;
    descriptorWrites.push_back(irradianceWrite);

    VkWriteDescriptorSet prefilterWrite{};
    prefilterWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    prefilterWrite.dstSet = targetSet;
    prefilterWrite.dstBinding = 2;
    prefilterWrite.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    prefilterWrite.descriptorCount = 1;
    prefilterWrite.pImageInfo = &prefilterInfo;
    descriptorWrites.push_back(prefilterWrite);

    VkWriteDescriptorSet brdfWrite{};
    brdfWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    brdfWrite.dstSet = targetSet;
    brdfWrite.dstBinding = 3;
    brdfWrite.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    brdfWrite.descriptorCount = 1;
    brdfWrite.pImageInfo = &brdfInfo;
    descriptorWrites.push_back(brdfWrite);

    vkUpdateDescriptorSets(renderData.rdVkbDevice.device, static_cast<uint32_t>(descriptorWrites.size()),
                           descriptorWrites.data(), 0, nullptr);

}

bool Core::Renderer::VkRenderer::createDebugSkeletonPipelineLayout()
{
    auto& renderData = Engine::getInstance().getRenderData();

    auto pipelineLayoutConfig = PipelineLayoutConfig{};
    pipelineLayoutConfig.setLayouts = {renderData.rdGlobalSceneDescriptorLayout};

    if (!PipelineLayout::init(renderData, renderData.rdDebugSkeletonPipelineLayout, pipelineLayoutConfig))
    {
        Logger::log(1, "%s error: could not init debug skeleton pipeline layout\n", __FUNCTION__);
        return false;
    }

    return true;
}

bool Core::Renderer::VkRenderer::createPrimitivePipeline()
{
    auto& renderData = Engine::getInstance().getRenderData();

    constexpr std::string_view vertexShaderFile = "shaders/primitive.vert.spv";
    constexpr std::string_view fragmentShaderFile = "shaders/primitive.frag.spv";
    if (!MeshPipelineLayout::init(renderData, renderData.rdMeshPipelineLayout))
    {
        Logger::log(1, "%s error: could not init mesh pipeline layout\n", __FUNCTION__);
        return false;
    }

    PipelineConfig pipelineConfig{};
    pipelineConfig.enableBlending = VK_TRUE;
    pipelineConfig.enableDepthTest = VK_TRUE;
    pipelineConfig.enableDepthWrite = VK_TRUE;
    pipelineConfig.depthCompareOp = VK_COMPARE_OP_LESS;

    if (!Pipeline::init(renderData, renderData.rdMeshPipelineLayout, renderData.rdMeshPipeline,
                       VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST, vertexShaderFile.data(),
                       fragmentShaderFile.data(), pipelineConfig))
    {
        Logger::log(1, "%s error: could not init mesh pipeline\n", __FUNCTION__);
        return false;
    }

    return true;
}

bool Core::Renderer::VkRenderer::createDebugSkeletonPipeline()
{
    const std::string vertexShaderFile = "shaders/debug_line.vert.spv";
    const std::string fragmentShaderFile = "shaders/debug_line.frag.spv";
    if (!DebugSkeletonPipeline::init(Engine::getInstance().getRenderData(),
                                     Engine::getInstance().getRenderData().rdDebugSkeletonPipelineLayout,
                                     Engine::getInstance().getRenderData().rdDebugSkeletonPipeline,
                                     VK_PRIMITIVE_TOPOLOGY_LINE_LIST, vertexShaderFile, fragmentShaderFile))
    {
        Logger::log(1, "%s error: could not init debug skeleton shader pipeline\n", __FUNCTION__);
        return false;
    }

    return true;
}

bool Core::Renderer::VkRenderer::createSkyboxPipelineLayout()
{
    auto& renderData = Engine::getInstance().getRenderData();

    auto pipelineLayoutConfig = PipelineLayoutConfig{};
    pipelineLayoutConfig.setLayouts = {renderData.rdGlobalSceneDescriptorLayout, renderData.rdSkyboxData.descriptorSetLayout};

    if (!PipelineLayout::init(renderData, renderData.rdSkyboxPipelineLayout, pipelineLayoutConfig))
    {
        Logger::log(1, "%s error: could not init skybox pipeline layout\n", __FUNCTION__);
        return false;
    }

    return true;
}

bool Core::Renderer::VkRenderer::createSkyboxPipeline()
{
    const std::string vertexShaderFile = "shaders/skybox.vert.spv";
    const std::string fragmentShaderFile = "shaders/skybox.frag.spv";

    auto& renderData = Engine::getInstance().getRenderData();

    if (!Pipeline::init(renderData, renderData.rdSkyboxPipelineLayout, renderData.rdSkyboxPipeline,
                        VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST, vertexShaderFile, fragmentShaderFile))
    {
        Logger::log(1, "%s error: could not init cubemap pipeline\n", __FUNCTION__);
        return false;
    }

    return true;
}

bool Core::Renderer::VkRenderer::loadSkybox()
{
    auto& renderData = Engine::getInstance().getRenderData();

    std::future<bool> hdrTextureFuture = Texture::loadHDRTexture(renderData, renderData.rdHDRTexture,
        "hdr/skybox.hdr");
    if (!hdrTextureFuture.get())
    {
        Logger::log(1, "%s error: could not load HDR texture", __FUNCTION__);
        return false;
    }

    initCaptureResources();

    if (!IBLGenerator::init(renderData))
    {
        return false;
    }

    if (!IBLGenerator::generateIBL(renderData))
    {
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

    Logger::log(1, "%s: cubemap loaded successfully\n", __FUNCTION__);

    return true;
}

void Core::Renderer::VkRenderer::drawSkybox() const
{
    auto& renderData = Engine::getInstance().getRenderData();

    if (renderData.rdSkyboxPipeline == VK_NULL_HANDLE || renderData.rdSkyboxData.descriptorSet == VK_NULL_HANDLE)
    {
        return;
    }

    vkCmdBindPipeline(renderData.rdCommandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, renderData.rdSkyboxPipeline);

    std::vector descriptorSets = {renderData.rdGlobalSceneUBO.rdUBODescriptorSet, renderData.rdSkyboxData.descriptorSet};

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

void Core::Renderer::VkRenderer::updateGlobalSceneData()
{
    auto& renderData = Engine::getInstance().getRenderData();

    mGlobalSceneData.view = mCamera.getViewMatrix(renderData);
    mGlobalSceneData.projection = glm::perspective(glm::radians(static_cast<float>(renderData.rdFieldOfView)),
                                                      static_cast<float>(renderData.rdVkbSwapchain.extent.width) /
                                                          static_cast<float>(renderData.rdVkbSwapchain.extent.height),
                                                      0.01f, 50.0f);

    mGlobalSceneData.camPos = glm::vec4(renderData.rdCameraWorldPosition, 1.0f);

    // dummy light. will be replaced later with real lights from the scene
    mGlobalSceneData.lightCount = glm::ivec4(1, 0, 0, 0);
    mGlobalSceneData.lightPositions[0] = glm::vec4(0.f, 10.f, 10.f, 1.f);
    mGlobalSceneData.lightColors[0] = glm::vec4(1.f, 1.f, 1.f, 1.f);

    UniformBuffer::uploadData(renderData, renderData.rdGlobalSceneUBO, mGlobalSceneData);
}

void Core::Renderer::VkRenderer::handleCameraMovementKeys()
{
    ImGuiIO& io = ImGui::GetIO();
    if (io.WantCaptureKeyboard)
    {
        return;
    }

    VkRenderData& renderData = Engine::getInstance().getRenderData();
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
