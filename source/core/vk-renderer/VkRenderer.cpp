#define VMA_IMPLEMENTATION
#define GLM_ENABLE_EXPERIMENTAL
#include "vk_mem_alloc.h"
#include "VkRenderer.h"
#include "core/tools/Logger.h"
#include "Framebuffer.h"
#include "Renderpass.h"
#include "PipelineLayout.h"
#include "core/vk-renderer/pipelines/Pipeline.h"
#include "CommandPool.h"
#include "core/vk-renderer/buffers/CommandBuffer.h"
#include "SyncObjects.h"
#include "Texture.h"
#include "core/vk-renderer/buffers/UniformBuffer.h"
#include "imgui.h"
#include "core/vk-renderer/buffers/VertexBuffer.h"
#include "core/events/input-events/MouseMovementEvent.h"
#include "core/vk-renderer/pipelines/GltfPipeline.h"
#include "core/vk-renderer/pipelines/GltfSkeletonPipeline.h"
#include "core/vk-renderer/buffers/ShaderStorageBuffer.h"
#include "core/vk-renderer/pipelines/GltfGPUPipeline.h"
#include "core/utils/ShapeUtils.h"
#include "core/vk-renderer/pipelines/MeshPipeline.h"

#include <core/events/input-events/MouseLockEvent.h>
#include <glm/gtc/matrix_transform.hpp>

Core::Renderer::VkRenderer::VkRenderer(GLFWwindow* inWindow)
{
    mRenderData.rdWindow = inWindow;
    mPerspectiveViewMatrices.emplace_back(1.0f);
    mPerspectiveViewMatrices.emplace_back(1.0f);
}

bool Core::Renderer::VkRenderer::init(const unsigned int width, const unsigned int height)
{
    if (!mRenderData.rdWindow)
    {
        Logger::log(1, "%s error: Can't init Vulkan mCore::Renderer. mRenderData.rdWindow is invalid\n", __FUNCTION__);
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

    if (!createUBO())
    {
        return false;
    }

    // should be loaded before ssbo to properly init joint matrices
    if (!loadGltfModel())
    {
        return false;
    }

    if (!createSSBO())
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

    if (!createGltfPipelineLayout())
    {
        return false;
    }

    if (!createGltfPipeline())
    {
        return false;
    }

    if (!createGltfSkeletonPipeline())
    {
        return false;
    }

    if (!createGltfGPUPipeline())
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

    if (!initUserInterface())
    {
        return false;
    }

    mRenderData.rdWidth = static_cast<int>(width);
    mRenderData.rdHeight = static_cast<int>(height);

    mModel = std::make_unique<Core::Model::Model>();

    mEulerModelMesh = std::make_unique<Core::Renderer::VkMesh>();
    mQuaternionModelMesh = std::make_unique<Core::Renderer::VkMesh>();
    Logger::log(1, "%s: model mesh storage initialized\n", __FUNCTION__);

    mSkeletonMesh = std::make_shared<Core::Renderer::VkMesh>();
    Logger::log(1, "%s: skeleton mesh storage initialized\n", __FUNCTION__);

    mAllMeshes = std::make_unique<Core::Renderer::VkMesh>();
    Logger::log(1, "%s: global mesh storage initialized\n", __FUNCTION__);

    mFrameTimer.start();

    Logger::log(1, "%s: Vulkan Core::Renderer initialized to %ix%i\n", __FUNCTION__, width, height);
    return true;
}

void Core::Renderer::VkRenderer::setSize(unsigned int width, unsigned int height)
{
    mRenderData.rdWidth = width;
    mRenderData.rdHeight = height;

    Logger::log(1, "%s: resized window to %ix%i\n", __FUNCTION__, width, height);
}

// maybe move to some global functions?
void Core::Renderer::VkRenderer::subscribeToInputEvents(EventDispatcher& eventDispatcher)
{
    eventDispatcher.subscribe(this);
}

void Core::Renderer::VkRenderer::onEvent(const Event& event)
{
    if (const auto* mouseMovementEvent = dynamic_cast<const MouseMovementEvent*>(&event))
    {
        mRenderData.rdViewYaw += static_cast<float>(mouseMovementEvent->deltaX) / 10.f;
        if (mRenderData.rdViewYaw < 0.f)
        {
            mRenderData.rdViewYaw += 360.f;
        }
        if (mRenderData.rdViewYaw >= 360.f)
        {
            mRenderData.rdViewYaw -= 360.f;
        }

        mRenderData.rdViewPitch -= static_cast<float>(mouseMovementEvent->deltaY) / 10.f;
        if (mRenderData.rdViewPitch > 89.f)
        {
            mRenderData.rdViewPitch = 89.f;
        }
        if (mRenderData.rdViewPitch < -89.f)
        {
            mRenderData.rdViewPitch = -89.f;
        }
    }
    else if (const auto* mouseLockEvent = dynamic_cast<const MouseLockEvent*>(&event))
    {
        if (mouseLockEvent->isLocked)
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
}

bool Core::Renderer::VkRenderer::draw()
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

    VkClearValue colorClearValue;
    colorClearValue.color = {{0.f, 0.f, 0.f, 1.0f}};

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
    viewport.y = static_cast<float>(mRenderData.rdVkbSwapchain.extent.height);
    viewport.width = static_cast<float>(mRenderData.rdVkbSwapchain.extent.width);
    /* invert viewport from OpenGL */
    viewport.height = -static_cast<float>(mRenderData.rdVkbSwapchain.extent.height);
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;

    VkRect2D scissor{};
    scissor.offset = {0, 0};
    scissor.extent = mRenderData.rdVkbSwapchain.extent;

    mMatrixGenerateTimer.start();
    mPerspectiveViewMatrices.at(0) = mCamera.getViewMatrix(mRenderData);
    mPerspectiveViewMatrices.at(1) = glm::perspective(glm::radians(static_cast<float>(mRenderData.rdFieldOfView)),
                                                      static_cast<float>(mRenderData.rdVkbSwapchain.extent.width) /
                                                          static_cast<float>(mRenderData.rdVkbSwapchain.extent.height),
                                                      0.01f, 50.0f);
#if 0
    if (mRenderData.rdDrawSkeleton)
    {
        mSkeletonMesh = mGltfModel->getSkeleton(true);
    }
#endif

    // TODO
    // fix after ubo changes
    /* mPerspectiveViewMatrices.lightPosition = mRenderData.rdLightPosition;
    mPerspectiveViewMatrices.lightColor = mRenderData.rdLightColor;*/

    mRenderData.rdMatrixGenerateTime = mMatrixGenerateTimer.stop();

    if (mRenderData.rdResetAngles)
    {
        mRenderData.rdResetAngles = false;

        mRenderData.rdRotXAngle = 0;
        mRenderData.rdRotYAngle = 0;
        mRenderData.rdRotZAngle = 0;

        mEulerRotMatrix = glm::mat3(1.f);
        mQuaternionModelOrientation = glm::quat();
    }

    mRotYMat = glm::rotate(glm::mat4(1.0f), glm::radians(static_cast<float>(mRenderData.rdRotYAngle)), mRotYAxis);
    mRotZMat = glm::rotate(mRotYMat, glm::radians(static_cast<float>(mRenderData.rdRotZAngle)), mRotZAxis);
    mEulerRotMatrix = glm::rotate(mRotZMat, glm::radians(static_cast<float>(mRenderData.rdRotXAngle)), mRotXAxis);

    mQuaternionModelOrientation =
        glm::normalize(glm::quat(glm::vec3(glm::radians(static_cast<float>(mRenderData.rdRotXAngle)),
                                           glm::radians(static_cast<float>(mRenderData.rdRotYAngle)),
                                           glm::radians(static_cast<float>(mRenderData.rdRotZAngle)))));

    mQuaternionModelOrientationConjugate = glm::conjugate(mQuaternionModelOrientation);

    mGridMesh.vertices.clear();

    mCoordinateArrowsMesh.vertices.clear();

    if (mRenderData.rdDrawWorldCoordinateArrows)
    {
        mCoordinateArrowsMesh = mCoordinateArrowsModel.getVertexData();
        std::for_each(mCoordinateArrowsMesh.vertices.begin(), mCoordinateArrowsMesh.vertices.end(),
                      [this](VkVertex& vertex) { vertex.color /= 2.f; });

        mAllMeshes->vertices.insert(mAllMeshes->vertices.end(), mCoordinateArrowsMesh.vertices.begin(),
                                    mCoordinateArrowsMesh.vertices.end());
    }

    mEulerCoordinateArrowsMesh.vertices.clear();
    mQuaternionArrowMesh.vertices.clear();

    if (mRenderData.rdDrawModelCoordinateArrows)
    {
        mEulerCoordinateArrowsMesh = mCoordinateArrowsModel.getVertexData();
        std::for_each(mEulerCoordinateArrowsMesh.vertices.begin(), mEulerCoordinateArrowsMesh.vertices.end(),
                      [this](VkVertex& vertex)
                      {
                          vertex.position = mEulerRotMatrix * vertex.position;
                          vertex.position += mEulerModelDist;
                      });

        mAllMeshes->vertices.insert(mAllMeshes->vertices.end(), mEulerCoordinateArrowsMesh.vertices.begin(),
                                    mEulerCoordinateArrowsMesh.vertices.end());

        mQuaternionArrowMesh = mArrowModel.getVertexData();
        std::for_each(mQuaternionArrowMesh.vertices.begin(), mQuaternionArrowMesh.vertices.end(),
                      [this](VkVertex& vertex)
                      {
                          glm::quat position = glm::quat(0.f, vertex.position.x, vertex.position.y, vertex.position.z);
                          glm::quat newPosition =
                              mQuaternionModelOrientation * position * mQuaternionModelOrientationConjugate;
                          vertex.position.x = newPosition.x;
                          vertex.position.y = newPosition.y;
                          vertex.position.z = newPosition.z;
                          vertex.position += mQuaternionModelDist;
                      });
        mAllMeshes->vertices.insert(mAllMeshes->vertices.end(), mQuaternionArrowMesh.vertices.begin(),
                                    mQuaternionArrowMesh.vertices.end());
    }

    if (mRenderData.rdDrawSkeleton)
    {
        mAllMeshes->vertices.insert(mAllMeshes->vertices.end(), mSkeletonMesh->vertices.begin(),
                                    mSkeletonMesh->vertices.end());
    }

    *mEulerModelMesh = mModel->getVertexData();
    mRenderData.rdTriangleCount = mEulerModelMesh->vertices.size() / 3;
    std::for_each(mEulerModelMesh->vertices.begin(), mEulerModelMesh->vertices.end(),
                  [this](VkVertex& vertex)
                  {
                      vertex.position = mEulerRotMatrix * vertex.position;
                      vertex.position += mEulerModelDist;
                  });
    mAllMeshes->vertices.insert(mAllMeshes->vertices.end(), mEulerModelMesh->vertices.begin(),
                                mEulerModelMesh->vertices.end());

    *mQuaternionModelMesh = mModel->getVertexData();
    mRenderData.rdTriangleCount += mQuaternionModelMesh->vertices.size() / 3;
    std::for_each(mQuaternionModelMesh->vertices.begin(), mQuaternionModelMesh->vertices.end(),
                  [this](VkVertex& vertex)
                  {
                      glm::quat position = glm::quat(0.f, vertex.position.x, vertex.position.y, vertex.position.z);
                      glm::quat newPosition =
                          mQuaternionModelOrientation * position * mQuaternionModelOrientationConjugate;
                      vertex.position.x = newPosition.x;
                      vertex.position.y = newPosition.y;
                      vertex.position.z = newPosition.z;
                      vertex.position += mQuaternionModelDist;
                  });
    mAllMeshes->vertices.insert(mAllMeshes->vertices.end(), mQuaternionModelMesh->vertices.begin(),
                                mQuaternionModelMesh->vertices.end());

    mLineIndexCount = mCoordinateArrowsMesh.vertices.size() + mEulerCoordinateArrowsMesh.vertices.size() +
                      mQuaternionArrowMesh.vertices.size();

    mSkeletonLineIndexCount = mRenderData.rdDrawSkeleton ? mSkeletonMesh->vertices.size() : 0;

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

    mUploadToVBOTimer.start();

    Core::Renderer::VertexBuffer::uploadData(mRenderData, mRenderData.rdVertexBufferData, *mAllMeshes);

    /* upload required data only when switching GPU and CPU */
    static bool lastGPURenderState = mRenderData.rdGPUVertexSkinning;

    if (lastGPURenderState != mRenderData.rdGPUVertexSkinning)
    {
        mModelUploadRequired = true;
        lastGPURenderState = mRenderData.rdGPUVertexSkinning;
    }

#if 0
    if (mModelUploadRequired)
    {
        mGltfModel->uploadVertexBuffers(mRenderData, mGltfRenderData);
        mGltfModel->uploadIndexBuffer(mRenderData, mGltfRenderData);
        mModelUploadRequired = false;
    }

    if (!mRenderData.rdGPUVertexSkinning)
    {

        /* glTF vertex skinning, overwrites position buffer, needs upload on every frame */
        mGltfModel->applyVertexSkinning(mRenderData, mGltfRenderData);
    }
#endif
    mPrimitive->uploadVertexBuffers(mRenderData, mPrimitiveRenderData);
    mPrimitive->uploadIndexBuffer(mRenderData, mPrimitiveRenderData);

    mRenderData.rdUploadToVBOTime = mUploadToVBOTimer.stop();

    vkCmdBeginRenderPass(mRenderData.rdCommandBuffer, &rpInfo, VK_SUBPASS_CONTENTS_INLINE);

    vkCmdSetViewport(mRenderData.rdCommandBuffer, 0, 1, &viewport);
    vkCmdSetScissor(mRenderData.rdCommandBuffer, 0, 1, &scissor);

    vkCmdBindDescriptorSets(mRenderData.rdCommandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, mRenderData.rdPipelineLayout,
                            0, 1, &mRenderData.rdModelTexture.texTextureDescriptorSet, 0, nullptr);

    vkCmdBindDescriptorSets(mRenderData.rdCommandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, mRenderData.rdPipelineLayout,
                            1, 1, &mRenderData.rdPerspectiveViewMatrixUBO.rdUBODescriptorSet, 0, nullptr);

    vkCmdBindDescriptorSets(mRenderData.rdCommandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, mRenderData.rdPipelineLayout,
                            2, 1, &mRenderData.rdJointMatrixSSBO.rdSSBODescriptorSet, 0, nullptr);

    VkDeviceSize offset = 0;
    vkCmdBindVertexBuffers(mRenderData.rdCommandBuffer, 0, 1, &mRenderData.rdVertexBufferData.rdVertexBuffer, &offset);

    if (mLineIndexCount > 0)
    {
        vkCmdBindPipeline(mRenderData.rdCommandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, mRenderData.rdLinePipeline);
        vkCmdSetLineWidth(mRenderData.rdCommandBuffer, 3.0f);
        vkCmdDraw(mRenderData.rdCommandBuffer, mLineIndexCount, 1, 0, 0);
    }

    // draw box
    vkCmdBindPipeline(mRenderData.rdCommandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, mRenderData.rdBasicPipeline);
    vkCmdDraw(mRenderData.rdCommandBuffer, mRenderData.rdTriangleCount * 3, 1,
              mLineIndexCount + mSkeletonLineIndexCount, 0);

    // draw grid
    vkCmdBindPipeline(mRenderData.rdCommandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, mRenderData.rdGridPipeline);
    vkCmdDraw(mRenderData.rdCommandBuffer, 6, 1, 0, 0);

#if 0
    // draw glTF model
    if (mRenderData.rdDrawGltfModel)
    {
        mGltfModel->draw(mRenderData, mGltfRenderData);
    }

    // draw skeleton
    if (mSkeletonLineIndexCount > 0 && mRenderData.rdDrawSkeleton)
    {
        vkCmdBindVertexBuffers(mRenderData.rdCommandBuffer, 0, 1, &mRenderData.rdVertexBufferData.rdVertexBuffer,
                               &offset);

        vkCmdBindPipeline(mRenderData.rdCommandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS,
                          mRenderData.rdGltfSkeletonPipeline);
        vkCmdSetLineWidth(mRenderData.rdCommandBuffer, 3.0f);
        vkCmdDraw(mRenderData.rdCommandBuffer, mSkeletonLineIndexCount, 1, mLineIndexCount, 0);
    }
#endif

    mPrimitive->draw(mRenderData, mPrimitiveRenderData);

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

    Core::Renderer::UniformBuffer::uploadData(mRenderData, mRenderData.rdPerspectiveViewMatrixUBO,
                                              mPerspectiveViewMatrices);

    Core::Renderer::ShaderStorageBuffer::uploadData(mRenderData, mRenderData.rdJointMatrixSSBO,
                                                    mGltfModel->getJointMatrices());

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

void Core::Renderer::VkRenderer::cleanup()
{
    vkDeviceWaitIdle(mRenderData.rdVkbDevice.device);

#if 0
    mGltfModel->cleanup(mRenderData, mGltfRenderData);
    mGltfModel.reset();
#endif

    mUserInterface.cleanup(mRenderData);

    Core::Renderer::SyncObjects::cleanup(mRenderData);
    Core::Renderer::CommandBuffer::cleanup(mRenderData, mRenderData.rdCommandBuffer);
    Core::Renderer::CommandPool::cleanup(mRenderData);
    Core::Renderer::Framebuffer::cleanup(mRenderData);
    Core::Renderer::GltfGPUPipeline::cleanup(mRenderData, mRenderData.rdGltfGPUPipeline);
    Core::Renderer::GltfSkeletonPipeline::cleanup(mRenderData, mRenderData.rdGltfSkeletonPipeline);
    Core::Renderer::GltfPipeline::cleanup(mRenderData, mRenderData.rdMeshPipeline);
    Core::Renderer::GltfPipeline::cleanup(mRenderData, mRenderData.rdGltfPipeline);
    Core::Renderer::Pipeline::cleanup(mRenderData, mRenderData.rdGridPipeline);
    Core::Renderer::Pipeline::cleanup(mRenderData, mRenderData.rdLinePipeline);
    Core::Renderer::Pipeline::cleanup(mRenderData, mRenderData.rdBasicPipeline);
    Core::Renderer::PipelineLayout::cleanup(mRenderData, mRenderData.rdPipelineLayout);
    Core::Renderer::PipelineLayout::cleanup(mRenderData, mRenderData.rdGltfPipelineLayout);
    Core::Renderer::Renderpass::cleanup(mRenderData);
    Core::Renderer::UniformBuffer::cleanup(mRenderData, mRenderData.rdPerspectiveViewMatrixUBO);
    Core::Renderer::ShaderStorageBuffer::cleanup(mRenderData, mRenderData.rdJointMatrixSSBO);
    Core::Renderer::VertexBuffer::cleanup(mRenderData, mRenderData.rdVertexBufferData);
    Core::Renderer::Texture::cleanup(mRenderData, mRenderData.rdModelTexture);

    vkDestroyImageView(mRenderData.rdVkbDevice.device, mRenderData.rdDepthImageView, nullptr);
    vmaDestroyImage(mRenderData.rdAllocator, mRenderData.rdDepthImage, mRenderData.rdDepthImageAlloc);
    vmaDestroyAllocator(mRenderData.rdAllocator);

    mRenderData.rdVkbSwapchain.destroy_image_views(mRenderData.rdSwapchainImageViews);
    vkb::destroy_swapchain(mRenderData.rdVkbSwapchain);

    vkb::destroy_device(mRenderData.rdVkbDevice);
    vkb::destroy_surface(mRenderData.rdVkbInstance.instance, mSurface);
    vkb::destroy_instance(mRenderData.rdVkbInstance);

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

bool Core::Renderer::VkRenderer::getQueue()
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

bool Core::Renderer::VkRenderer::createSwapchain()
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

bool Core::Renderer::VkRenderer::createDepthBuffer()
{
    const VkExtent3D depthImageExtent = {mRenderData.rdVkbSwapchain.extent.width,
                                         mRenderData.rdVkbSwapchain.extent.height, 1};

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
    depthAllocInfo.requiredFlags = static_cast<VkMemoryPropertyFlags>(VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

    if (vmaCreateImage(mRenderData.rdAllocator, &depthImageInfo, &depthAllocInfo, &mRenderData.rdDepthImage,
                       &mRenderData.rdDepthImageAlloc, nullptr) != VK_SUCCESS)
    {
        Logger::log(1, "%s error: could not allocate depth buffer memory\n", __FUNCTION__);
        return false;
    }

    VkImageViewCreateInfo depthImageViewInfo{};
    depthImageViewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    depthImageViewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
    depthImageViewInfo.image = mRenderData.rdDepthImage;
    depthImageViewInfo.format = mRenderData.rdDepthFormat;
    depthImageViewInfo.subresourceRange.baseMipLevel = 0;
    depthImageViewInfo.subresourceRange.levelCount = 1;
    depthImageViewInfo.subresourceRange.baseArrayLayer = 0;
    depthImageViewInfo.subresourceRange.layerCount = 1;
    depthImageViewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;

    if (vkCreateImageView(mRenderData.rdVkbDevice.device, &depthImageViewInfo, nullptr,
                          &mRenderData.rdDepthImageView) != VK_SUCCESS)
    {
        Logger::log(1, "%s error: could not create depth buffer image view\n", __FUNCTION__);
        return false;
    }
    return true;
}

bool Core::Renderer::VkRenderer::recreateSwapchain()
{
    while (mRenderData.rdWidth == 0 || mRenderData.rdHeight == 0)
    {
        glfwGetFramebufferSize(mRenderData.rdWindow, &mRenderData.rdWidth, &mRenderData.rdHeight);
        glfwWaitEvents();
    }

    vkDeviceWaitIdle(mRenderData.rdVkbDevice.device);

    /* cleanup */
    Core::Renderer::Framebuffer::cleanup(mRenderData);
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

bool Core::Renderer::VkRenderer::createUBO()
{
    size_t matrixSize = mPerspectiveViewMatrices.size() * sizeof(glm::mat4);

    if (!Core::Renderer::UniformBuffer::init(mRenderData, mRenderData.rdPerspectiveViewMatrixUBO, matrixSize))
    {
        Logger::log(1, "%s error: could not create uniform buffers\n", __FUNCTION__);
        return false;
    }
    return true;
}

bool Core::Renderer::VkRenderer::createSSBO()
{
    size_t matrixSize = mGltfModel->getJointMatrices().size() * sizeof(glm::mat4);

    if (!Core::Renderer::ShaderStorageBuffer::init(mRenderData, mRenderData.rdJointMatrixSSBO, matrixSize))
    {
        Logger::log(1, "%s error: could not create uniform buffers\n", __FUNCTION__);
        return false;
    }
    return true;
}

bool Core::Renderer::VkRenderer::createVBO()
{
    if (!Core::Renderer::VertexBuffer::init(mRenderData, mRenderData.rdVertexBufferData, VertexBufferSize))
    {
        Logger::log(1, "%s error: could not create vertex buffer\n", __FUNCTION__);
        return false;
    }
    return true;
}

bool Core::Renderer::VkRenderer::createRenderPass()
{
    if (!Core::Renderer::Renderpass::init(mRenderData))
    {
        Logger::log(1, "%s error: could not init renderpass\n", __FUNCTION__);
        return false;
    }
    return true;
}

bool Core::Renderer::VkRenderer::createPipelineLayout()
{
    if (!Core::Renderer::PipelineLayout::init(mRenderData, mRenderData.rdModelTexture, mRenderData.rdPipelineLayout))
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
    if (!Pipeline::init(mRenderData, mRenderData.rdPipelineLayout, mRenderData.rdBasicPipeline,
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
    if (!Pipeline::init(mRenderData, mRenderData.rdPipelineLayout, mRenderData.rdLinePipeline,
                        VK_PRIMITIVE_TOPOLOGY_LINE_LIST, vertexShaderFile, fragmentShaderFile))
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
    if (!Pipeline::init(mRenderData, mRenderData.rdPipelineLayout, mRenderData.rdGridPipeline,
                        VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST, vertexShaderFile, fragmentShaderFile))
    {
        Logger::log(1, "%s error: could not init grid shader pipeline\n", __FUNCTION__);
        return false;
    }
    return true;
}

bool Core::Renderer::VkRenderer::createGltfPipelineLayout()
{
    if (!Core::Renderer::PipelineLayout::init(mRenderData, mRenderData.rdModelTexture,
                                              mRenderData.rdGltfPipelineLayout))
    {
        Logger::log(1, "%s error: could not init gltf pipeline layout\n", __FUNCTION__);
        return false;
    }
    return true;
}

bool Core::Renderer::VkRenderer::createGltfPipeline()
{
    const std::string vertexShaderFile = "shaders/gltf.vert.spv";
    const std::string fragmentShaderFile = "shaders/gltf.frag.spv";
    if (!GltfPipeline::init(mRenderData, mRenderData.rdGltfPipelineLayout, mRenderData.rdGltfPipeline,
                            VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST, vertexShaderFile, fragmentShaderFile))
    {
        Logger::log(1, "%s error: could not init gltf shader pipeline\n", __FUNCTION__);
        return false;
    }
    return true;
}

bool Core::Renderer::VkRenderer::createGltfSkeletonPipeline()
{
    const std::string vertexShaderFile = "shaders/line.vert.spv";
    const std::string fragmentShaderFile = "shaders/line.frag.spv";
    if (!GltfSkeletonPipeline::init(mRenderData, mRenderData.rdGltfPipelineLayout, mRenderData.rdGltfSkeletonPipeline,
                                    VK_PRIMITIVE_TOPOLOGY_LINE_LIST, vertexShaderFile, fragmentShaderFile))
    {
        Logger::log(1, "%s error: could not init gltf skeleton shader pipeline\n", __FUNCTION__);
        return false;
    }
    return true;
}

bool Core::Renderer::VkRenderer::createGltfGPUPipeline()
{
    const std::string vertexShaderFile = "shaders/gltf_gpu.vert.spv";
    const std::string fragmentShaderFile = "shaders/gltf_gpu.frag.spv";
    if (!GltfGPUPipeline::init(mRenderData, mRenderData.rdPipelineLayout, mRenderData.rdGltfGPUPipeline,
                               VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST, vertexShaderFile, fragmentShaderFile))
    {
        Logger::log(1, "%s error: could not init gltf GPU shader pipeline\n", __FUNCTION__);
        return false;
    }
    return true;
}

bool Core::Renderer::VkRenderer::createFramebuffer()
{
    if (!Core::Renderer::Framebuffer::init(mRenderData))
    {
        Logger::log(1, "%s error: could not init framebuffer\n", __FUNCTION__);
        return false;
    }
    return true;
}

bool Core::Renderer::VkRenderer::createCommandPool()
{
    if (!Core::Renderer::CommandPool::init(mRenderData))
    {
        Logger::log(1, "%s error: could not create command pool\n", __FUNCTION__);
        return false;
    }
    return true;
}

bool Core::Renderer::VkRenderer::createCommandBuffer()
{
    if (!Core::Renderer::CommandBuffer::init(mRenderData, mRenderData.rdCommandBuffer))
    {
        Logger::log(1, "%s error: could not create command buffers\n", __FUNCTION__);
        return false;
    }
    return true;
}

bool Core::Renderer::VkRenderer::createSyncObjects()
{
    if (!Core::Renderer::SyncObjects::init(mRenderData))
    {
        Logger::log(1, "%s error: could not create sync objects\n", __FUNCTION__);
        return false;
    }

    return true;
}

bool Core::Renderer::VkRenderer::loadTexture()
{
    const std::string textureFileName = "textures/default.png";
    std::future<bool> textureLoadFuture =
        Core::Renderer::Texture::loadTexture(mRenderData, mRenderData.rdModelTexture, textureFileName);
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

bool Core::Renderer::VkRenderer::initUserInterface()
{
    if (!mUserInterface.init(mRenderData))
    {
        Logger::log(1, "%s error: could not init ImGui\n", __FUNCTION__);
        return false;
    }
    return true;
}

bool Core::Renderer::VkRenderer::loadGltfModel()
{
    mGltfModel = std::make_shared<Core::Model::GltfModel>();
    const std::string modelFilename = "assets/Woman.gltf";
    const std::string modelTexFilename = "textures/Woman.png";
    if (!mGltfModel->loadModel(mRenderData, mGltfRenderData, modelFilename, modelTexFilename))
    {
        Logger::log(1, "%s: loading glTF model '%s' failed\n", __FUNCTION__, modelFilename.c_str());
        return false;
    }
    return true;
}

bool Core::Renderer::VkRenderer::loadMeshWithAssimp()
{
    const std::string vertexShaderFile = "shaders/mesh.vert.spv";
    const std::string fragmentShaderFile = "shaders/mesh.frag.spv";
    if (!MeshPipeline::init(mRenderData, mRenderData.rdPipelineLayout, mRenderData.rdMeshPipeline,
                            VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST, vertexShaderFile, fragmentShaderFile))
    {
        Logger::log(1, "%s error: could not init mesh pipeline\n", __FUNCTION__);
        return false;
    }

    const std::string modelFileName = "assets/girl/scene.gltf";
    Core::Utils::ShapeData primitiveShapeData = Core::Utils::loadShapeFromFile(modelFileName, mRenderData);
    std::vector<Core::Renderer::NewVertex> primitiveVertices =
        Core::Utils::getVerticesFromShapeData(primitiveShapeData);
    std::vector<uint32_t> primitiveIndices = Core::Utils::getIndicesFromShapeData(primitiveShapeData);
    std::vector<Core::Renderer::VkTextureData> primitiveTexture = primitiveShapeData.textures;

    mPrimitive = std::make_shared<Core::Renderer::Primitive>("Box", primitiveVertices, primitiveIndices,
                                                             primitiveTexture, mRenderData, mPrimitiveRenderData);

    return true;
}

void Core::Renderer::VkRenderer::handleWindowMoveEvents(int xPosition, int yPosition)
{
    Logger::log(1, "%s: mRenderData.rdWindow has been moved to %i/%i\n", __FUNCTION__, xPosition, yPosition);
}

void Core::Renderer::VkRenderer::handleWindowMinimizedEvents(int minimized)
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

void Core::Renderer::VkRenderer::handleWindowMaximizedEvents(int maximized)
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

void Core::Renderer::VkRenderer::handleWindowCloseEvents()
{
    Logger::log(1, "%s: mRenderData.rdWindow has been closed\n", __FUNCTION__);
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
        mRenderData.rdMoveUp += 1.f;
    }
    if (glfwGetKey(mRenderData.rdWindow, GLFW_KEY_Q) == GLFW_PRESS)
    {
        mRenderData.rdMoveUp -= 1.f;
    }
}
