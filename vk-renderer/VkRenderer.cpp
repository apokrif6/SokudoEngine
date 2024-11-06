#define VMA_IMPLEMENTATION
#include "vk_mem_alloc.h"
#include "VkRenderer.h"
#include "Logger.h"
#include "Framebuffer.h"
#include "Renderpass.h"
#include "PipelineLayout.h"
#include "Pipeline.h"
#include "CommandPool.h"
#include "CommandBuffer.h"
#include "SyncObjects.h"
#include "Texture.h"
#include "UniformBuffer.h"
#include "imgui.h"
#include "VertexBuffer.h"
#include <glm/gtc/matrix_transform.hpp>

VkRenderer::VkRenderer(GLFWwindow* inWindow)
{
    mRenderData.rdWindow = inWindow;
    mMatrices.viewMatrix = glm::mat4(1.0f);
    mMatrices.projectionMatrix = glm::mat4(1.0f);
}

bool VkRenderer::init(unsigned int width, unsigned int height)
{
    if (!mRenderData.rdWindow)
    {
        Logger::log(1, "%s error: Can't init Vulkan mRenderer. mRenderData.rdWindow is invalid\n", __FUNCTION__);
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

    if (!loadTexture())
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

    if (!createFramebuffer())
    {
        return false;
    }

    if (!createSyncObjects())
    {
        return false;
    }

    if (!initUserInterface())
    {
        return false;
    }

    mRenderData.rdWidth = static_cast<int>(width);
    mRenderData.rdHeight = static_cast<int>(height);

    mModel = std::make_unique<Model>();

    mEulerModelMesh = std::make_unique<VkMesh>();
    Logger::log(1, "%s: model mesh storage initialized\n", __FUNCTION__);

    mAllMeshes = std::make_unique<VkMesh>();
    Logger::log(1, "%s: global mesh storage initialized\n", __FUNCTION__);

    mFrameTimer.start();

    Logger::log(1, "%s: Vulkan renderer initialized to %ix%i\n", __FUNCTION__, width, height);
    return true;
}

void VkRenderer::setSize(unsigned int width, unsigned int height)
{
    mRenderData.rdWidth = width;
    mRenderData.rdHeight = height;

    Logger::log(1, "%s: resized window to %ix%i\n", __FUNCTION__, width, height);
}

bool VkRenderer::draw()
{
    double tickTime = glfwGetTime();
    mRenderData.rdTickDiff = static_cast<float>(tickTime - mLastTickTime);

    mRenderData.rdFrameTime = mFrameTimer.stop();
    mFrameTimer.start();

    handleCameraMovementKeys();

    mAllMeshes->vertices.clear();

    if (vkWaitForFences(mRenderData.rdVkbDevice.device, 1, &mRenderData.rdRenderFence, VK_TRUE, UINT64_MAX) !=
        VK_SUCCESS)
    {
        Logger::log(1, "%s error: waiting for fence failed\n", __FUNCTION__);
        return false;
    }

    if (vkResetFences(mRenderData.rdVkbDevice.device, 1, &mRenderData.rdRenderFence) != VK_SUCCESS)
    {
        Logger::log(1, "%s error: fence reset failed\n", __FUNCTION__);
        return false;
    }

    uint32_t imageIndex = 0;
    VkResult result = vkAcquireNextImageKHR(mRenderData.rdVkbDevice.device, mRenderData.rdVkbSwapchain.swapchain,
                                            UINT64_MAX, mRenderData.rdPresentSemaphore, VK_NULL_HANDLE, &imageIndex);

    if (result == VK_ERROR_OUT_OF_DATE_KHR)
    {
        return recreateSwapchain();
    }
    else
    {
        if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR)
        {
            Logger::log(1, "%s error: failed to acquire swapchain image. Error is '%i'\n", __FUNCTION__, result);
            return false;
        }
    }

    if (vkResetCommandBuffer(mRenderData.rdCommandBuffer, 0) != VK_SUCCESS)
    {
        Logger::log(1, "%s error: failed to reset command buffer\n", __FUNCTION__);
        return false;
    }

    VkCommandBufferBeginInfo cmdBeginInfo{};
    cmdBeginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    cmdBeginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

    if (vkBeginCommandBuffer(mRenderData.rdCommandBuffer, &cmdBeginInfo) != VK_SUCCESS)
    {
        Logger::log(1, "%s error: failed to begin command buffer\n", __FUNCTION__);
        return false;
    }

    VkClearValue colorClearValue;
    colorClearValue.color = {{0.1f, 0.1f, 0.1f, 1.0f}};

    VkClearValue depthValue;
    depthValue.depthStencil.depth = 1.0f;

    VkClearValue clearValues[] = {colorClearValue, depthValue};

    VkRenderPassBeginInfo rpInfo{};
    rpInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    rpInfo.renderPass = mRenderData.rdRenderpass;

    rpInfo.renderArea.offset.x = 0;
    rpInfo.renderArea.offset.y = 0;
    rpInfo.renderArea.extent = mRenderData.rdVkbSwapchain.extent;
    rpInfo.framebuffer = mRenderData.rdFramebuffers[imageIndex];

    rpInfo.clearValueCount = 2;
    rpInfo.pClearValues = clearValues;

    VkViewport viewport{};
    viewport.x = 0.0f;
    viewport.y = 0.0f;
    viewport.width = static_cast<float>(mRenderData.rdVkbSwapchain.extent.width);
    viewport.height = static_cast<float>(mRenderData.rdVkbSwapchain.extent.height);
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;

    VkRect2D scissor{};
    scissor.offset = {0, 0};
    scissor.extent = mRenderData.rdVkbSwapchain.extent;

    mMatrixGenerateTimer.start();
    mMatrices.projectionMatrix = glm::perspective(glm::radians(static_cast<float>(mRenderData.rdFieldOfView)),
                                                  static_cast<float>(mRenderData.rdVkbSwapchain.extent.width) /
                                                      static_cast<float>(mRenderData.rdVkbSwapchain.extent.height),
                                                  0.1f, 10.0f);

    mMatrices.viewMatrix = mCamera.getViewMatrix(mRenderData);
    mRenderData.rdMatrixGenerateTime = mMatrixGenerateTimer.stop();

    if (mRenderData.rdResetAngles)
    {
        mRenderData.rdResetAngles = false;

        mRenderData.rdRotXAngle = 0;
        mRenderData.rdRotYAngle = 0;
        mRenderData.rdRotZAngle = 0;
    }

    mRotYMat = glm::rotate(glm::mat4(1.0f), glm::radians(static_cast<float>(mRenderData.rdRotYAngle)), mRotYAxis);
    mRotZMat = glm::rotate(mRotYMat, glm::radians(static_cast<float>(mRenderData.rdRotZAngle)), mRotZAxis);
    mEulerRotMatrix = glm::rotate(mRotZMat, glm::radians(static_cast<float>(mRenderData.rdRotXAngle)), mRotXAxis);

    mCoordArrowsMesh.vertices.clear();
    mEulerCoordArrowsMesh.vertices.clear();

    if (mRenderData.rdDrawModelCoordArrows)
    {
        mEulerCoordArrowsMesh = mCoordinateArrowsModel.getVertexData();
        std::for_each(mEulerCoordArrowsMesh.vertices.begin(), mEulerCoordArrowsMesh.vertices.end(),
                      [this](VkVertex& n)
                      {
                          n.position = mEulerRotMatrix * n.position;
                          n.position += mEulerModelDist;
                      });
        mAllMeshes->vertices.insert(mAllMeshes->vertices.end(), mEulerCoordArrowsMesh.vertices.begin(),
                                    mEulerCoordArrowsMesh.vertices.end());
    }

    *mEulerModelMesh = mModel->getVertexData();
    mRenderData.rdTriangleCount = mEulerModelMesh->vertices.size() / 3;
    std::for_each(mEulerModelMesh->vertices.begin(), mEulerModelMesh->vertices.end(),
                  [this](VkVertex& n)
                  {
                      n.position = mEulerRotMatrix * n.position;
                      n.position += mEulerModelDist;
                  });
    mAllMeshes->vertices.insert(mAllMeshes->vertices.end(), mEulerModelMesh->vertices.begin(),
                                mEulerModelMesh->vertices.end());

    mLineIndexCount = mCoordArrowsMesh.vertices.size() + mEulerCoordArrowsMesh.vertices.size();

    mUploadToVBOTimer.start();
    VertexBuffer::uploadData(mRenderData, *mAllMeshes);
    mRenderData.rdUploadToVBOTime = mUploadToVBOTimer.stop();

    vkCmdBeginRenderPass(mRenderData.rdCommandBuffer, &rpInfo, VK_SUBPASS_CONTENTS_INLINE);

    vkCmdSetViewport(mRenderData.rdCommandBuffer, 0, 1, &viewport);
    vkCmdSetScissor(mRenderData.rdCommandBuffer, 0, 1, &scissor);

    VkDeviceSize offset = 0;
    vkCmdBindVertexBuffers(mRenderData.rdCommandBuffer, 0, 1, &mRenderData.rdVertexBuffer, &offset);

    vkCmdBindDescriptorSets(mRenderData.rdCommandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, mRenderData.rdPipelineLayout,
                            0, 1, &mRenderData.rdTextureDescriptorSet, 0, nullptr);
    vkCmdBindDescriptorSets(mRenderData.rdCommandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, mRenderData.rdPipelineLayout,
                            1, 1, &mRenderData.rdUBODescriptorSet, 0, nullptr);

    if (mLineIndexCount > 0)
    {
        vkCmdBindPipeline(mRenderData.rdCommandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, mRenderData.rdLinePipeline);
        vkCmdSetLineWidth(mRenderData.rdCommandBuffer, 3.0f);
        vkCmdDraw(mRenderData.rdCommandBuffer, mLineIndexCount, 1, 0, 0);
    }

    vkCmdBindPipeline(mRenderData.rdCommandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, mRenderData.rdBasicPipeline);
    vkCmdDraw(mRenderData.rdCommandBuffer, mRenderData.rdTriangleCount * 3, 1, mLineIndexCount, 0);

    mUIGenerateTimer.start();
    mUserInterface.createFrame(mRenderData);
    mRenderData.rdUIGenerateTime = mUIGenerateTimer.stop();

    mUIDrawTimer.start();
    mUserInterface.render(mRenderData);
    mRenderData.rdUIDrawTime = mUIDrawTimer.stop();

    vkCmdEndRenderPass(mRenderData.rdCommandBuffer);

    if (vkEndCommandBuffer(mRenderData.rdCommandBuffer) != VK_SUCCESS)
    {
        Logger::log(1, "%s error: failed to end command buffer\n", __FUNCTION__);
        return false;
    }

    mUploadToUBOTimer.start();
    UniformBuffer::uploadData(mRenderData, mMatrices);
    mRenderData.rdUploadToUBOTime = mUploadToUBOTimer.stop();

    VkSubmitInfo submitInfo{};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

    VkPipelineStageFlags waitStage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    submitInfo.pWaitDstStageMask = &waitStage;

    submitInfo.waitSemaphoreCount = 1;
    submitInfo.pWaitSemaphores = &mRenderData.rdPresentSemaphore;

    submitInfo.signalSemaphoreCount = 1;
    submitInfo.pSignalSemaphores = &mRenderData.rdRenderSemaphore;

    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &mRenderData.rdCommandBuffer;

    if (vkQueueSubmit(mRenderData.rdGraphicsQueue, 1, &submitInfo, mRenderData.rdRenderFence) != VK_SUCCESS)
    {
        Logger::log(1, "%s error: failed to submit draw command buffer\n", __FUNCTION__);
        return false;
    }

    VkPresentInfoKHR presentInfo{};
    presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
    presentInfo.waitSemaphoreCount = 1;
    presentInfo.pWaitSemaphores = &mRenderData.rdRenderSemaphore;

    presentInfo.swapchainCount = 1;
    presentInfo.pSwapchains = &mRenderData.rdVkbSwapchain.swapchain;

    presentInfo.pImageIndices = &imageIndex;

    result = vkQueuePresentKHR(mRenderData.rdPresentQueue, &presentInfo);
    if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR)
    {
        return recreateSwapchain();
    }
    else
    {
        if (result != VK_SUCCESS)
        {
            Logger::log(1, "%s error: failed to present swapchain image\n", __FUNCTION__);
            return false;
        }
    }

    mLastTickTime = tickTime;

    return true;
}

void VkRenderer::cleanup()
{
    vkDeviceWaitIdle(mRenderData.rdVkbDevice.device);

    mUserInterface.cleanup(mRenderData);

    Texture::cleanup(mRenderData);
    vmaDestroyBuffer(mRenderData.rdAllocator, mVertexBuffer, mVertexBufferAlloc);

    SyncObjects::cleanup(mRenderData);
    CommandBuffer::cleanup(mRenderData, mRenderData.rdCommandBuffer);
    CommandPool::cleanup(mRenderData);
    Framebuffer::cleanup(mRenderData);
    Pipeline::cleanup(mRenderData, mRenderData.rdLinePipeline);
    Pipeline::cleanup(mRenderData, mRenderData.rdBasicPipeline);
    PipelineLayout::cleanup(mRenderData, mRenderData.rdPipelineLayout);
    Renderpass::cleanup(mRenderData);
    UniformBuffer::cleanup(mRenderData);
    VertexBuffer::cleanup(mRenderData);

    vkDestroyImageView(mRenderData.rdVkbDevice.device, mRenderData.rdDepthImageView, nullptr);
    vmaDestroyImage(mRenderData.rdAllocator, mRenderData.rdDepthImage, mRenderData.rdDepthImageAlloc);
    vmaDestroyAllocator(mRenderData.rdAllocator);

    mRenderData.rdVkbSwapchain.destroy_image_views(mRenderData.rdSwapchainImageViews);
    vkb::destroy_swapchain(mRenderData.rdVkbSwapchain);

    vkb::destroy_device(mRenderData.rdVkbDevice);
    vkb::destroy_surface(mRenderData.rdVkbInstance.instance, mSurface);
    vkb::destroy_instance(mRenderData.rdVkbInstance);

    Logger::log(1, "%s: Vulkan renderer destroyed\n", __FUNCTION__);
}

bool VkRenderer::deviceInit()
{
    vkb::InstanceBuilder instBuild;
    auto instRet = instBuild.use_default_debug_messenger().request_validation_layers().build();
    if (!instRet)
    {
        Logger::log(1, "%s error: could not build vkb instance\n", __FUNCTION__);
        return false;
    }
    mRenderData.rdVkbInstance = instRet.value();

    VkResult result = VK_ERROR_UNKNOWN;
    result = glfwCreateWindowSurface(mRenderData.rdVkbInstance, mRenderData.rdWindow, nullptr, &mSurface);
    if (result != VK_SUCCESS)
    {
        Logger::log(1, "%s error: Could not create Vulkan surface\n", __FUNCTION__);
        return false;
    }

    /* just get the first available device */
    vkb::PhysicalDeviceSelector physicalDevSel{mRenderData.rdVkbInstance};
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

    mRenderData.rdVkbPhysicalDevice = secondPhysicalDevSelRet.value();

    Logger::log(1, "%s: found physical device '%s'\n", __FUNCTION__, mRenderData.rdVkbPhysicalDevice.name.c_str());

    mMinUniformBufferOffsetAlignment =
        mRenderData.rdVkbPhysicalDevice.properties.limits.minUniformBufferOffsetAlignment;
    Logger::log(1, "%s: the physical device as a minimal uniform buffer offset of %i bytes\n", __FUNCTION__,
                mMinUniformBufferOffsetAlignment);

    vkb::DeviceBuilder devBuilder{mRenderData.rdVkbPhysicalDevice};
    auto devBuilderRet = devBuilder.build();
    if (!devBuilderRet)
    {
        Logger::log(1, "%s error: could not get devices\n", __FUNCTION__);
        return false;
    }
    mRenderData.rdVkbDevice = devBuilderRet.value();

    return true;
}

bool VkRenderer::getQueue()
{
    auto graphQueueRet = mRenderData.rdVkbDevice.get_queue(vkb::QueueType::graphics);
    if (!graphQueueRet.has_value())
    {
        Logger::log(1, "%s error: could not get graphics queue\n", __FUNCTION__);
        return false;
    }
    mRenderData.rdGraphicsQueue = graphQueueRet.value();

    auto presentQueueRet = mRenderData.rdVkbDevice.get_queue(vkb::QueueType::present);
    if (!presentQueueRet.has_value())
    {
        Logger::log(1, "%s error: could not get present queue\n", __FUNCTION__);
        return false;
    }
    mRenderData.rdPresentQueue = presentQueueRet.value();

    return true;
}

bool VkRenderer::createSwapchain()
{
    vkb::SwapchainBuilder swapChainBuild{mRenderData.rdVkbDevice};

    glfwGetFramebufferSize(mRenderData.rdWindow, &mRenderData.rdWidth, &mRenderData.rdHeight);

    auto swapChainBuildRet = swapChainBuild.set_old_swapchain(mRenderData.rdVkbSwapchain)
                                 .set_desired_present_mode(VK_PRESENT_MODE_FIFO_KHR)
                                 .set_desired_extent(mRenderData.rdWidth, mRenderData.rdHeight)
                                 .build();
    if (!swapChainBuildRet)
    {
        Logger::log(1, "%s error: could not init swapchain\n", __FUNCTION__);
        return false;
    }

    vkb::destroy_swapchain(mRenderData.rdVkbSwapchain);
    mRenderData.rdVkbSwapchain = swapChainBuildRet.value();

    return true;
}

bool VkRenderer::createDepthBuffer()
{
    VkExtent3D depthImageExtent = {mRenderData.rdVkbSwapchain.extent.width, mRenderData.rdVkbSwapchain.extent.height,
                                   1};

    mRenderData.rdDepthFormat = VK_FORMAT_D32_SFLOAT;

    VkImageCreateInfo depthImageInfo{};
    depthImageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    depthImageInfo.imageType = VK_IMAGE_TYPE_2D;
    depthImageInfo.format = mRenderData.rdDepthFormat;
    depthImageInfo.extent = depthImageExtent;
    depthImageInfo.mipLevels = 1;
    depthImageInfo.arrayLayers = 1;
    depthImageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
    depthImageInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
    depthImageInfo.usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;

    VmaAllocationCreateInfo depthAllocInfo{};
    depthAllocInfo.usage = VMA_MEMORY_USAGE_GPU_ONLY;
    depthAllocInfo.requiredFlags = VkMemoryPropertyFlags(VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

    if (vmaCreateImage(mRenderData.rdAllocator, &depthImageInfo, &depthAllocInfo, &mRenderData.rdDepthImage,
                       &mRenderData.rdDepthImageAlloc, nullptr) != VK_SUCCESS)
    {
        Logger::log(1, "%s error: could not allocate depth buffer memory\n", __FUNCTION__);
        return false;
    }

    VkImageViewCreateInfo depthImageViewinfo{};
    depthImageViewinfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    depthImageViewinfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
    depthImageViewinfo.image = mRenderData.rdDepthImage;
    depthImageViewinfo.format = mRenderData.rdDepthFormat;
    depthImageViewinfo.subresourceRange.baseMipLevel = 0;
    depthImageViewinfo.subresourceRange.levelCount = 1;
    depthImageViewinfo.subresourceRange.baseArrayLayer = 0;
    depthImageViewinfo.subresourceRange.layerCount = 1;
    depthImageViewinfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;

    if (vkCreateImageView(mRenderData.rdVkbDevice.device, &depthImageViewinfo, nullptr,
                          &mRenderData.rdDepthImageView) != VK_SUCCESS)
    {
        Logger::log(1, "%s error: could not create depth buffer image view\n", __FUNCTION__);
        return false;
    }
    return true;
}

bool VkRenderer::recreateSwapchain()
{
    while (mRenderData.rdWidth == 0 || mRenderData.rdHeight == 0)
    {
        glfwGetFramebufferSize(mRenderData.rdWindow, &mRenderData.rdWidth, &mRenderData.rdHeight);
        glfwWaitEvents();
    }

    vkDeviceWaitIdle(mRenderData.rdVkbDevice.device);

    /* cleanup */
    Framebuffer::cleanup(mRenderData);
    vkDestroyImageView(mRenderData.rdVkbDevice.device, mRenderData.rdDepthImageView, nullptr);
    vmaDestroyImage(mRenderData.rdAllocator, mRenderData.rdDepthImage, mRenderData.rdDepthImageAlloc);

    mRenderData.rdVkbSwapchain.destroy_image_views(mRenderData.rdSwapchainImageViews);

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

bool VkRenderer::createUBO()
{
    if (!UniformBuffer::init(mRenderData))
    {
        Logger::log(1, "%s error: could not create uniform buffers\n", __FUNCTION__);
        return false;
    }
    return true;
}

bool VkRenderer::createVBO()
{
    if (!VertexBuffer::init(mRenderData))
    {
        Logger::log(1, "%s error: could not create vertex buffer\n", __FUNCTION__);
        return false;
    }
    return true;
}

bool VkRenderer::createRenderPass()
{
    if (!Renderpass::init(mRenderData))
    {
        Logger::log(1, "%s error: could not init renderpass\n", __FUNCTION__);
        return false;
    }
    return true;
}

bool VkRenderer::createPipelineLayout()
{
    if (!PipelineLayout::init(mRenderData, mRenderData.rdPipelineLayout))
    {
        Logger::log(1, "%s error: could not init pipeline layout\n", __FUNCTION__);
        return false;
    }
    return true;
}

bool VkRenderer::createBasicPipeline()
{
    std::string vertexShaderFile = "shaders/basic.vert.spv";
    std::string fragmentShaderFile = "shaders/basic.frag.spv";
    if (!Pipeline::init(mRenderData, mRenderData.rdPipelineLayout, mRenderData.rdBasicPipeline,
                        VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST, vertexShaderFile, fragmentShaderFile))
    {
        Logger::log(1, "%s error: could not init pipeline\n", __FUNCTION__);
        return false;
    }
    return true;
}

bool VkRenderer::createLinePipeline()
{
    std::string vertexShaderFile = "shaders/line.vert.spv";
    std::string fragmentShaderFile = "shaders/line.frag.spv";
    if (!Pipeline::init(mRenderData, mRenderData.rdPipelineLayout, mRenderData.rdLinePipeline,
                        VK_PRIMITIVE_TOPOLOGY_LINE_LIST, vertexShaderFile, fragmentShaderFile))
    {
        Logger::log(1, "%s error: could not init line shader pipeline\n", __FUNCTION__);
        return false;
    }
    return true;
}

bool VkRenderer::createFramebuffer()
{
    if (!Framebuffer::init(mRenderData))
    {
        Logger::log(1, "%s error: could not init framebuffer\n", __FUNCTION__);
        return false;
    }
    return true;
}

bool VkRenderer::createCommandPool()
{
    if (!CommandPool::init(mRenderData))
    {
        Logger::log(1, "%s error: could not create command pool\n", __FUNCTION__);
        return false;
    }
    return true;
}

bool VkRenderer::createCommandBuffer()
{
    if (!CommandBuffer::init(mRenderData, mRenderData.rdCommandBuffer))
    {
        Logger::log(1, "%s error: could not create command buffers\n", __FUNCTION__);
        return false;
    }
    return true;
}

bool VkRenderer::createSyncObjects()
{
    if (!SyncObjects::init(mRenderData))
    {
        Logger::log(1, "%s error: could not create sync objects\n", __FUNCTION__);
        return false;
    }

    return true;
}

bool VkRenderer::loadTexture()
{
    std::string textureFileName = "textures/default.png";
    if (!Texture::loadTexture(mRenderData, textureFileName))
    {
        Logger::log(1, "%s error: could not load texture\n", __FUNCTION__);
        return false;
    }

    return true;
}

bool VkRenderer::initVma()
{
    VmaAllocatorCreateInfo allocatorInfo{};
    allocatorInfo.physicalDevice = mRenderData.rdVkbPhysicalDevice.physical_device;
    allocatorInfo.device = mRenderData.rdVkbDevice.device;
    allocatorInfo.instance = mRenderData.rdVkbInstance.instance;
    if (vmaCreateAllocator(&allocatorInfo, &mRenderData.rdAllocator) != VK_SUCCESS)
    {
        Logger::log(1, "%s error: could not init VMA\n", __FUNCTION__);
        return false;
    }

    return true;
}

bool VkRenderer::initUserInterface()
{
    if (!mUserInterface.init(mRenderData))
    {
        Logger::log(1, "%s error: could not init ImGui\n", __FUNCTION__);
        return false;
    }
    return true;
}

void VkRenderer::handleWindowMoveEvents(int xPosition, int yPosition)
{
    Logger::log(1, "%s: mRenderData.rdWindow has been moved to %i/%i\n", __FUNCTION__, xPosition, yPosition);
}

void VkRenderer::handleWindowMinimizedEvents(int minimized)
{
    if (minimized)
    {
        Logger::log(1, "%s: mRenderData.rdWindow has been minimized\n", __FUNCTION__);
    }
    else
    {
        Logger::log(1, "%s: mRenderData.rdWindow has been restored\n", __FUNCTION__);
    }
}

void VkRenderer::handleWindowMaximizedEvents(int maximized)
{
    if (maximized)
    {
        Logger::log(1, "%s: mRenderData.rdWindow has been maximized\n", __FUNCTION__);
    }
    else
    {
        Logger::log(1, "%s: mRenderData.rdWindow has been restored\n", __FUNCTION__);
    }
}

void VkRenderer::handleWindowCloseEvents()
{
    Logger::log(1, "%s: mRenderData.rdWindow has been closed\n", __FUNCTION__);
}

void VkRenderer::handleKeyEvents(int key, int scancode, int action, int mods)
{
    std::string actionName;
    switch (action)
    {
    case GLFW_PRESS:
        actionName = "pressed";
        break;
    case GLFW_RELEASE:
        actionName = "released";
        break;
    case GLFW_REPEAT:
        actionName = "repeated";
        break;
    default:
        actionName = "invalid";
        break;
    }

    const char* keyName = glfwGetKeyName(key, 0);
    Logger::log(1, "%s: key %s (key %i, scancode %i) %s\n", __FUNCTION__, keyName, key, scancode, actionName.c_str());
}

void VkRenderer::handleMouseButtonEvents(int button, int action, int mods)
{
    ImGuiIO& io = ImGui::GetIO();
    if (button >= 0 && button < ImGuiMouseButton_COUNT)
    {
        io.AddMouseButtonEvent(button, action == GLFW_PRESS);
    }

    if (button == GLFW_MOUSE_BUTTON_RIGHT && action == GLFW_PRESS)
    {
        mMouseLock = !mMouseLock;

        if (mMouseLock)
        {
            glfwSetInputMode(mRenderData.rdWindow, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
            if (glfwRawMouseMotionSupported())
            {
                glfwSetInputMode(mRenderData.rdWindow, GLFW_RAW_MOUSE_MOTION, GLFW_TRUE);
            }
        }
        else
        {
            glfwSetInputMode(mRenderData.rdWindow, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
        }
    }

    std::string actionName;
    switch (action)
    {
    case GLFW_PRESS:
        actionName = "pressed";
        break;
    case GLFW_RELEASE:
        actionName = "released";
        break;
    case GLFW_REPEAT:
        actionName = "repeated";
        break;
    default:
        actionName = "invalid";
        break;
    }

    std::string mouseButtonName;
    switch (button)
    {
    case GLFW_MOUSE_BUTTON_LEFT:
        mouseButtonName = "left";
        break;
    case GLFW_MOUSE_BUTTON_MIDDLE:
        mouseButtonName = "middle";
        break;
    case GLFW_MOUSE_BUTTON_RIGHT:
        mouseButtonName = "right";
        break;
    default:
        mouseButtonName = "other";
        break;
    }

    Logger::log(1, "%s: %s mouse button (%i) %s\n", __FUNCTION__, mouseButtonName.c_str(), button, actionName.c_str());
}

void VkRenderer::handleMousePositionEvents(double xPosition, double yPosition)
{
    ImGuiIO& io = ImGui::GetIO();
    io.AddMousePosEvent(static_cast<float>(xPosition), static_cast<float>(yPosition));

    if (io.WantCaptureMouse)
    {
        return;
    }

    int mouseMoveRelX = static_cast<int>(xPosition) - mMouseXPosition;
    int mouseMoveRelY = static_cast<int>(yPosition) - mMouseYPosition;

    if (mMouseLock)
    {
        mRenderData.rdViewYaw += static_cast<float>(mouseMoveRelX) / 10.f;
        if (mRenderData.rdViewYaw < 0.0)
        {
            mRenderData.rdViewYaw += 360.0;
        }
        if (mRenderData.rdViewYaw >= 360.0)
        {
            mRenderData.rdViewYaw -= 360.0;
        }

        mRenderData.rdViewPitch -= static_cast<float>(mouseMoveRelY) / 10.f;
        if (mRenderData.rdViewPitch > 89.0)
        {
            mRenderData.rdViewPitch = 89.0;
        }
        if (mRenderData.rdViewPitch < -89.0)
        {
            mRenderData.rdViewPitch = -89.0;
        }
    }

    mMouseXPosition = static_cast<int>(xPosition);
    mMouseYPosition = static_cast<int>(yPosition);

    Logger::log(1, "%s: Mouse cursor has been moved to %lf/%lf\n", __FUNCTION__, xPosition, yPosition);
}

void VkRenderer::handleMouseEnterLeaveEvents(int enter)
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

void VkRenderer::handleCameraMovementKeys()
{
    ImGuiIO& io = ImGui::GetIO();
    if (io.WantCaptureKeyboard)
    {
        return;
    }

    mRenderData.rdMoveForward = 0.f;
    if (glfwGetKey(mRenderData.rdWindow, GLFW_KEY_W) == GLFW_PRESS)
    {
        mRenderData.rdMoveForward += 1.f;
    }
    if (glfwGetKey(mRenderData.rdWindow, GLFW_KEY_S) == GLFW_PRESS)
    {
        mRenderData.rdMoveForward -= 1.f;
    }

    mRenderData.rdMoveRight = 0.f;
    if (glfwGetKey(mRenderData.rdWindow, GLFW_KEY_D) == GLFW_PRESS)
    {
        mRenderData.rdMoveRight += 1;
    }
    if (glfwGetKey(mRenderData.rdWindow, GLFW_KEY_A) == GLFW_PRESS)
    {
        mRenderData.rdMoveRight -= 1.f;
    }

    mRenderData.rdMoveUp = 0.f;
    if (glfwGetKey(mRenderData.rdWindow, GLFW_KEY_E) == GLFW_PRESS)
    {
        mRenderData.rdMoveUp -= 1.f;
    }
    if (glfwGetKey(mRenderData.rdWindow, GLFW_KEY_Q) == GLFW_PRESS)
    {
        mRenderData.rdMoveUp += 1.f;
    }
}
