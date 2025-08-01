#define VMA_IMPLEMENTATION
#define GLM_ENABLE_EXPERIMENTAL
#include "vk_mem_alloc.h"
#include "VkRenderer.h"
#include "core/tools/Logger.h"
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
#include <glm/gtc/matrix_transform.hpp>
#include "core/vk-renderer/pipelines/DebugSkeletonPipeline.h"
#include "core/vk-renderer/pipelines/layouts/DebugSkeletonPipelineLayout.h"
#include "core/engine/Engine.h"

bool Core::Renderer::VkRenderer::init(const unsigned int width, const unsigned int height)
{
    if (!Core::Engine::getInstance().getRenderData().rdWindow)
    {
        Logger::log(1, "%s error: Can't init Vulkan mCore::Renderer. Core::Engine::getInstance().getRenderData().rdWindow is invalid\n", __FUNCTION__);
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

    if (!initUserInterface())
    {
        return false;
    }

    Core::Engine::getInstance().getRenderData().rdWidth = static_cast<int>(width);
    Core::Engine::getInstance().getRenderData().rdHeight = static_cast<int>(height);

    mModel = std::make_unique<Core::Model::Model>();

    mEulerModelMesh = std::make_unique<Core::Renderer::VkMesh>();
    mQuaternionModelMesh = std::make_unique<Core::Renderer::VkMesh>();
    Logger::log(1, "%s: model mesh storage initialized\n", __FUNCTION__);

    mSkeletonMesh = std::make_shared<Core::Renderer::VkMesh>();
    Logger::log(1, "%s: skeleton mesh storage initialized\n", __FUNCTION__);

    mAllMeshes = std::make_unique<Core::Renderer::VkMesh>();
    Logger::log(1, "%s: global mesh storage initialized\n", __FUNCTION__);

    Logger::log(1, "%s: Vulkan Core::Renderer initialized to %ix%i\n", __FUNCTION__, width, height);
    return true;
}
Core::Renderer::VkRenderer::VkRenderer(GLFWwindow* inWindow)
{
    Core::Engine::getInstance().getRenderData().rdWindow = inWindow;
    mPerspectiveViewMatrices.emplace_back(1.0f);
    mPerspectiveViewMatrices.emplace_back(1.0f);
}

void Core::Renderer::VkRenderer::setSize(unsigned int width, unsigned int height)
{
    Core::Engine::getInstance().getRenderData().rdWidth = width;
    Core::Engine::getInstance().getRenderData().rdHeight = height;

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
        Core::Engine::getInstance().getRenderData().rdViewYaw += static_cast<float>(mouseMovementEvent->deltaX) / 10.f;
        if (Core::Engine::getInstance().getRenderData().rdViewYaw < 0.f)
        {
            Core::Engine::getInstance().getRenderData().rdViewYaw += 360.f;
        }
        if (Core::Engine::getInstance().getRenderData().rdViewYaw >= 360.f)
        {
            Core::Engine::getInstance().getRenderData().rdViewYaw -= 360.f;
        }

        Core::Engine::getInstance().getRenderData().rdViewPitch -= static_cast<float>(mouseMovementEvent->deltaY) / 10.f;
        if (Core::Engine::getInstance().getRenderData().rdViewPitch > 89.f)
        {
            Core::Engine::getInstance().getRenderData().rdViewPitch = 89.f;
        }
        if (Core::Engine::getInstance().getRenderData().rdViewPitch < -89.f)
        {
            Core::Engine::getInstance().getRenderData().rdViewPitch = -89.f;
        }
    }
    else if (const auto* mouseLockEvent = dynamic_cast<const MouseLockEvent*>(&event))
    {
        if (mouseLockEvent->isLocked)
        {
            glfwSetInputMode(Core::Engine::getInstance().getRenderData().rdWindow, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
            if (glfwRawMouseMotionSupported())
            {
                glfwSetInputMode(Core::Engine::getInstance().getRenderData().rdWindow, GLFW_RAW_MOUSE_MOTION, GLFW_TRUE);
            }
        }
        else
        {
            glfwSetInputMode(Core::Engine::getInstance().getRenderData().rdWindow, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
        }
    }
}
void Core::Renderer::VkRenderer::update(const VkRenderData& renderData, float deltaTime)
{
}

bool Core::Renderer::VkRenderer::draw()
{
    handleCameraMovementKeys();

    mAllMeshes->vertices.clear();

    if (vkWaitForFences(Core::Engine::getInstance().getRenderData().rdVkbDevice.device, 1, &Core::Engine::getInstance().getRenderData().rdRenderFence, VK_TRUE, UINT64_MAX) !=
        VK_SUCCESS)
    {
        Logger::log(1, "%s error: waiting for fence failed\n", __FUNCTION__);
        return false;
    }

    if (vkResetFences(Core::Engine::getInstance().getRenderData().rdVkbDevice.device, 1, &Core::Engine::getInstance().getRenderData().rdRenderFence) != VK_SUCCESS)
    {
        Logger::log(1, "%s error: fence reset failed\n", __FUNCTION__);
        return false;
    }

    uint32_t imageIndex = 0;
    VkResult result = vkAcquireNextImageKHR(Core::Engine::getInstance().getRenderData().rdVkbDevice.device, Core::Engine::getInstance().getRenderData().rdVkbSwapchain.swapchain,
                                            UINT64_MAX, Core::Engine::getInstance().getRenderData().rdPresentSemaphore, VK_NULL_HANDLE, &imageIndex);

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
    rpInfo.renderPass = Core::Engine::getInstance().getRenderData().rdRenderpass;

    rpInfo.renderArea.offset.x = 0;
    rpInfo.renderArea.offset.y = 0;
    rpInfo.renderArea.extent = Core::Engine::getInstance().getRenderData().rdVkbSwapchain.extent;
    rpInfo.framebuffer = Core::Engine::getInstance().getRenderData().rdFramebuffers[imageIndex];

    rpInfo.clearValueCount = 2;
    rpInfo.pClearValues = clearValues;

    VkViewport viewport{};
    viewport.x = 0.0f;
    viewport.y = static_cast<float>(Core::Engine::getInstance().getRenderData().rdVkbSwapchain.extent.height);
    viewport.width = static_cast<float>(Core::Engine::getInstance().getRenderData().rdVkbSwapchain.extent.width);
    /* invert viewport from OpenGL */
    viewport.height = -static_cast<float>(Core::Engine::getInstance().getRenderData().rdVkbSwapchain.extent.height);
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;

    VkRect2D scissor{};
    scissor.offset = {0, 0};
    scissor.extent = Core::Engine::getInstance().getRenderData().rdVkbSwapchain.extent;

    mMatrixGenerateTimer.start();
    mPerspectiveViewMatrices.at(0) = mCamera.getViewMatrix(Core::Engine::getInstance().getRenderData());
    mPerspectiveViewMatrices.at(1) = glm::perspective(glm::radians(static_cast<float>(Core::Engine::getInstance().getRenderData().rdFieldOfView)),
                                                      static_cast<float>(Core::Engine::getInstance().getRenderData().rdVkbSwapchain.extent.width) /
                                                          static_cast<float>(Core::Engine::getInstance().getRenderData().rdVkbSwapchain.extent.height),
                                                      0.01f, 50.0f);

    Core::Engine::getInstance().getRenderData().rdMatrixGenerateTime = mMatrixGenerateTimer.stop();

    if (Core::Engine::getInstance().getRenderData().rdResetAngles)
    {
        Core::Engine::getInstance().getRenderData().rdResetAngles = false;

        Core::Engine::getInstance().getRenderData().rdRotXAngle = 0;
        Core::Engine::getInstance().getRenderData().rdRotYAngle = 0;
        Core::Engine::getInstance().getRenderData().rdRotZAngle = 0;

        mEulerRotMatrix = glm::mat3(1.f);
        mQuaternionModelOrientation = glm::quat();
    }

    mRotYMat = glm::rotate(glm::mat4(1.0f), glm::radians(static_cast<float>(Core::Engine::getInstance().getRenderData().rdRotYAngle)), mRotYAxis);
    mRotZMat = glm::rotate(mRotYMat, glm::radians(static_cast<float>(Core::Engine::getInstance().getRenderData().rdRotZAngle)), mRotZAxis);
    mEulerRotMatrix = glm::rotate(mRotZMat, glm::radians(static_cast<float>(Core::Engine::getInstance().getRenderData().rdRotXAngle)), mRotXAxis);

    mQuaternionModelOrientation =
        glm::normalize(glm::quat(glm::vec3(glm::radians(static_cast<float>(Core::Engine::getInstance().getRenderData().rdRotXAngle)),
                                           glm::radians(static_cast<float>(Core::Engine::getInstance().getRenderData().rdRotYAngle)),
                                           glm::radians(static_cast<float>(Core::Engine::getInstance().getRenderData().rdRotZAngle)))));

    mQuaternionModelOrientationConjugate = glm::conjugate(mQuaternionModelOrientation);

    mGridMesh.vertices.clear();

    mCoordinateArrowsMesh.vertices.clear();

    if (Core::Engine::getInstance().getRenderData().rdDrawWorldCoordinateArrows)
    {
        mCoordinateArrowsMesh = mCoordinateArrowsModel.getVertexData();
        std::for_each(mCoordinateArrowsMesh.vertices.begin(), mCoordinateArrowsMesh.vertices.end(),
                      [this](VkVertex& vertex) { vertex.color /= 2.f; });

        mAllMeshes->vertices.insert(mAllMeshes->vertices.end(), mCoordinateArrowsMesh.vertices.begin(),
                                    mCoordinateArrowsMesh.vertices.end());
    }

    mEulerCoordinateArrowsMesh.vertices.clear();
    mQuaternionArrowMesh.vertices.clear();

    if (Core::Engine::getInstance().getRenderData().rdDrawModelCoordinateArrows)
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

    *mEulerModelMesh = mModel->getVertexData();
    Core::Engine::getInstance().getRenderData().rdTriangleCount = mEulerModelMesh->vertices.size() / 3;
    std::for_each(mEulerModelMesh->vertices.begin(), mEulerModelMesh->vertices.end(),
                  [this](VkVertex& vertex)
                  {
                      vertex.position = mEulerRotMatrix * vertex.position;
                      vertex.position += mEulerModelDist;
                  });
    mAllMeshes->vertices.insert(mAllMeshes->vertices.end(), mEulerModelMesh->vertices.begin(),
                                mEulerModelMesh->vertices.end());

    *mQuaternionModelMesh = mModel->getVertexData();
    Core::Engine::getInstance().getRenderData().rdTriangleCount += mQuaternionModelMesh->vertices.size() / 3;
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

    if (vkResetCommandBuffer(Core::Engine::getInstance().getRenderData().rdCommandBuffer, 0) != VK_SUCCESS)
    {
        Logger::log(1, "%s error: failed to reset command buffer\n", __FUNCTION__);
        return false;
    }

    VkCommandBufferBeginInfo cmdBeginInfo{};
    cmdBeginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    cmdBeginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

    if (vkBeginCommandBuffer(Core::Engine::getInstance().getRenderData().rdCommandBuffer, &cmdBeginInfo) != VK_SUCCESS)
    {
        Logger::log(1, "%s error: failed to begin command buffer\n", __FUNCTION__);
        return false;
    }

    mUploadToVBOTimer.start();

    Core::Renderer::VertexBuffer::uploadData(Core::Engine::getInstance().getRenderData(), Core::Engine::getInstance().getRenderData().rdVertexBufferData, *mAllMeshes);

    Core::Engine::getInstance().getSystem<Scene::Scene>()->update(Core::Engine::getInstance().getRenderData(), Core::Engine::getInstance().getRenderData().rdTickDiff);

    Core::Engine::getInstance().getRenderData().rdUploadToVBOTime = mUploadToVBOTimer.stop();

    vkCmdBeginRenderPass(Core::Engine::getInstance().getRenderData().rdCommandBuffer, &rpInfo, VK_SUBPASS_CONTENTS_INLINE);

    vkCmdSetViewport(Core::Engine::getInstance().getRenderData().rdCommandBuffer, 0, 1, &viewport);
    vkCmdSetScissor(Core::Engine::getInstance().getRenderData().rdCommandBuffer, 0, 1, &scissor);

    vkCmdBindDescriptorSets(Core::Engine::getInstance().getRenderData().rdCommandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, Core::Engine::getInstance().getRenderData().rdPipelineLayout,
                            0, 1, &Core::Engine::getInstance().getRenderData().rdModelTexture.texTextureDescriptorSet, 0, nullptr);

    vkCmdBindDescriptorSets(Core::Engine::getInstance().getRenderData().rdCommandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, Core::Engine::getInstance().getRenderData().rdPipelineLayout,
                            1, 1, &Core::Engine::getInstance().getRenderData().rdPerspectiveViewMatrixUBO.rdUBODescriptorSet, 0, nullptr);

    VkDeviceSize offset = 0;
    vkCmdBindVertexBuffers(Core::Engine::getInstance().getRenderData().rdCommandBuffer, 0, 1, &Core::Engine::getInstance().getRenderData().rdVertexBufferData.rdVertexBuffer, &offset);

    if (mLineIndexCount > 0)
    {
        vkCmdBindPipeline(Core::Engine::getInstance().getRenderData().rdCommandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, Core::Engine::getInstance().getRenderData().rdLinePipeline);
        vkCmdSetLineWidth(Core::Engine::getInstance().getRenderData().rdCommandBuffer, 3.0f);
        vkCmdDraw(Core::Engine::getInstance().getRenderData().rdCommandBuffer, mLineIndexCount, 1, 0, 0);
    }

    // draw box
    vkCmdBindPipeline(Core::Engine::getInstance().getRenderData().rdCommandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, Core::Engine::getInstance().getRenderData().rdBasicPipeline);
    vkCmdDraw(Core::Engine::getInstance().getRenderData().rdCommandBuffer, Core::Engine::getInstance().getRenderData().rdTriangleCount * 3, 1,
              mLineIndexCount + mSkeletonLineIndexCount, 0);

    // draw grid
    vkCmdBindPipeline(Core::Engine::getInstance().getRenderData().rdCommandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, Core::Engine::getInstance().getRenderData().rdGridPipeline);
    vkCmdDraw(Core::Engine::getInstance().getRenderData().rdCommandBuffer, 6, 1, 0, 0);

    // TODO
    // should be moved to Engine::Update
    Core::Engine::getInstance().getSystem<Scene::Scene>()->draw(Core::Engine::getInstance().getRenderData());

    mUIGenerateTimer.start();
    mUserInterface.createFrame(Core::Engine::getInstance().getRenderData());
    Core::Engine::getInstance().getRenderData().rdUIGenerateTime = mUIGenerateTimer.stop();

    mUIDrawTimer.start();
    mUserInterface.render(Core::Engine::getInstance().getRenderData());
    Core::Engine::getInstance().getRenderData().rdUIDrawTime = mUIDrawTimer.stop();

    vkCmdEndRenderPass(Core::Engine::getInstance().getRenderData().rdCommandBuffer);

    if (vkEndCommandBuffer(Core::Engine::getInstance().getRenderData().rdCommandBuffer) != VK_SUCCESS)
    {
        Logger::log(1, "%s error: failed to end command buffer\n", __FUNCTION__);
        return false;
    }

    mUploadToUBOTimer.start();

    Core::Renderer::UniformBuffer::uploadData(Core::Engine::getInstance().getRenderData(), Core::Engine::getInstance().getRenderData().rdPerspectiveViewMatrixUBO,
                                              mPerspectiveViewMatrices);

    Core::Engine::getInstance().getRenderData().rdUploadToUBOTime = mUploadToUBOTimer.stop();

    VkSubmitInfo submitInfo{};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

    VkPipelineStageFlags waitStage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    submitInfo.pWaitDstStageMask = &waitStage;

    submitInfo.waitSemaphoreCount = 1;
    submitInfo.pWaitSemaphores = &Core::Engine::getInstance().getRenderData().rdPresentSemaphore;

    submitInfo.signalSemaphoreCount = 1;
    submitInfo.pSignalSemaphores = &Core::Engine::getInstance().getRenderData().rdRenderSemaphore;

    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &Core::Engine::getInstance().getRenderData().rdCommandBuffer;

    if (vkQueueSubmit(Core::Engine::getInstance().getRenderData().rdGraphicsQueue, 1, &submitInfo, Core::Engine::getInstance().getRenderData().rdRenderFence) != VK_SUCCESS)
    {
        Logger::log(1, "%s error: failed to submit draw command buffer\n", __FUNCTION__);
        return false;
    }

    VkPresentInfoKHR presentInfo{};
    presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
    presentInfo.waitSemaphoreCount = 1;
    presentInfo.pWaitSemaphores = &Core::Engine::getInstance().getRenderData().rdRenderSemaphore;

    presentInfo.swapchainCount = 1;
    presentInfo.pSwapchains = &Core::Engine::getInstance().getRenderData().rdVkbSwapchain.swapchain;

    presentInfo.pImageIndices = &imageIndex;

    result = vkQueuePresentKHR(Core::Engine::getInstance().getRenderData().rdPresentQueue, &presentInfo);
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

    return true;
}

void Core::Renderer::VkRenderer::cleanup(VkRenderData& renderData)
{
    vkDeviceWaitIdle(Core::Engine::getInstance().getRenderData().rdVkbDevice.device);

    Core::Engine::getInstance().getSystem<Scene::Scene>()->cleanup(renderData);

    mUserInterface.cleanup(renderData);

    Core::Renderer::SyncObjects::cleanup(renderData);
    Core::Renderer::CommandBuffer::cleanup(renderData, Core::Engine::getInstance().getRenderData().rdCommandBuffer);
    Core::Renderer::CommandPool::cleanup(renderData);
    Core::Renderer::Framebuffer::cleanup(renderData);
    Core::Renderer::MeshPipeline::cleanup(renderData, renderData.rdMeshPipeline);
    Core::Renderer::DebugSkeletonPipeline::cleanup(renderData, Core::Engine::getInstance().getRenderData().rdDebugSkeletonPipeline);
    Core::Renderer::Pipeline::cleanup(renderData, Core::Engine::getInstance().getRenderData().rdGridPipeline);
    Core::Renderer::Pipeline::cleanup(renderData, renderData.rdLinePipeline);
    Core::Renderer::Pipeline::cleanup(renderData, renderData.rdBasicPipeline);
    Core::Renderer::PipelineLayout::cleanup(renderData, renderData.rdPipelineLayout);
    Core::Renderer::MeshPipelineLayout::cleanup(renderData, renderData.rdMeshPipelineLayout);
    Core::Renderer::DebugSkeletonPipelineLayout::cleanup(renderData, renderData.rdDebugSkeletonPipelineLayout);
    Core::Renderer::Renderpass::cleanup(renderData);
    Core::Renderer::UniformBuffer::cleanup(renderData, renderData.rdPerspectiveViewMatrixUBO);
    Core::Renderer::VertexBuffer::cleanup(renderData, renderData.rdVertexBufferData);
    Core::Renderer::Texture::cleanup(renderData, renderData.rdModelTexture);

    vkDestroyImageView(renderData.rdVkbDevice.device, renderData.rdDepthImageView, nullptr);
    vmaDestroyImage(renderData.rdAllocator, renderData.rdDepthImage, Core::Engine::getInstance().getRenderData().rdDepthImageAlloc);
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
    result = glfwCreateWindowSurface(Core::Engine::getInstance().getRenderData().rdVkbInstance, Core::Engine::getInstance().getRenderData().rdWindow, nullptr, &mSurface);
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

    Logger::log(1, "%s: found physical device '%s'\n", __FUNCTION__, Core::Engine::getInstance().getRenderData().rdVkbPhysicalDevice.name.c_str());

    mMinUniformBufferOffsetAlignment =
        Core::Engine::getInstance().getRenderData().rdVkbPhysicalDevice.properties.limits.minUniformBufferOffsetAlignment;
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

    glfwGetFramebufferSize(Core::Engine::getInstance().getRenderData().rdWindow, &Core::Engine::getInstance().getRenderData().rdWidth, &Core::Engine::getInstance().getRenderData().rdHeight);

    auto swapChainBuildRet = swapChainBuild.set_old_swapchain(Core::Engine::getInstance().getRenderData().rdVkbSwapchain)
                                 .set_desired_present_mode(VK_PRESENT_MODE_FIFO_KHR)
                                 .set_desired_extent(Core::Engine::getInstance().getRenderData().rdWidth, Core::Engine::getInstance().getRenderData().rdHeight)
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

    if (vmaCreateImage(Core::Engine::getInstance().getRenderData().rdAllocator, &depthImageInfo, &depthAllocInfo, &Core::Engine::getInstance().getRenderData().rdDepthImage,
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
    while (Core::Engine::getInstance().getRenderData().rdWidth == 0 || Core::Engine::getInstance().getRenderData().rdHeight == 0)
    {
        glfwGetFramebufferSize(Core::Engine::getInstance().getRenderData().rdWindow, &Core::Engine::getInstance().getRenderData().rdWidth, &Core::Engine::getInstance().getRenderData().rdHeight);
        glfwWaitEvents();
    }

    vkDeviceWaitIdle(Core::Engine::getInstance().getRenderData().rdVkbDevice.device);

    /* cleanup */
    Core::Renderer::Framebuffer::cleanup(Core::Engine::getInstance().getRenderData());
    vkDestroyImageView(Core::Engine::getInstance().getRenderData().rdVkbDevice.device, Core::Engine::getInstance().getRenderData().rdDepthImageView, nullptr);
    vmaDestroyImage(Core::Engine::getInstance().getRenderData().rdAllocator, Core::Engine::getInstance().getRenderData().rdDepthImage, Core::Engine::getInstance().getRenderData().rdDepthImageAlloc);

    Core::Engine::getInstance().getRenderData().rdVkbSwapchain.destroy_image_views(Core::Engine::getInstance().getRenderData().rdSwapchainImageViews);

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

    if (!Core::Renderer::UniformBuffer::init(Core::Engine::getInstance().getRenderData(), Core::Engine::getInstance().getRenderData().rdPerspectiveViewMatrixUBO, matrixSize))
    {
        Logger::log(1, "%s error: could not create uniform buffers\n", __FUNCTION__);
        return false;
    }
    return true;
}

bool Core::Renderer::VkRenderer::createVBO()
{
    if (!Core::Renderer::VertexBuffer::init(Core::Engine::getInstance().getRenderData(), Core::Engine::getInstance().getRenderData().rdVertexBufferData, VertexBufferSize))
    {
        Logger::log(1, "%s error: could not create vertex buffer\n", __FUNCTION__);
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
    if (!Core::Renderer::PipelineLayout::init(Core::Engine::getInstance().getRenderData(), Core::Engine::getInstance().getRenderData().rdModelTexture, Core::Engine::getInstance().getRenderData().rdPipelineLayout))
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
    if (!Pipeline::init(Core::Engine::getInstance().getRenderData(), Core::Engine::getInstance().getRenderData().rdPipelineLayout, Core::Engine::getInstance().getRenderData().rdBasicPipeline,
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
    if (!Pipeline::init(Core::Engine::getInstance().getRenderData(), Core::Engine::getInstance().getRenderData().rdPipelineLayout, Core::Engine::getInstance().getRenderData().rdLinePipeline,
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
    if (!Pipeline::init(Core::Engine::getInstance().getRenderData(), Core::Engine::getInstance().getRenderData().rdPipelineLayout, Core::Engine::getInstance().getRenderData().rdGridPipeline,
                        VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST, vertexShaderFile, fragmentShaderFile))
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
    if (!Core::Renderer::CommandBuffer::init(Core::Engine::getInstance().getRenderData(), Core::Engine::getInstance().getRenderData().rdCommandBuffer))
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

bool Core::Renderer::VkRenderer::loadTexture()
{
    const std::string textureFileName = "textures/default.png";
    std::future<bool> textureLoadFuture =
        Core::Renderer::Texture::loadTexture(Core::Engine::getInstance().getRenderData(), Core::Engine::getInstance().getRenderData().rdModelTexture, textureFileName);
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

bool Core::Renderer::VkRenderer::initUserInterface()
{
    if (!mUserInterface.init(Core::Engine::getInstance().getRenderData()))
    {
        Logger::log(1, "%s error: could not init ImGui\n", __FUNCTION__);
        return false;
    }
    return true;
}

bool Core::Renderer::VkRenderer::loadMeshWithAssimp()
{
    const std::string modelFileName = "assets/mixamo/models/FemaleModel.fbx";
    Core::Utils::MeshData primitiveMeshData = Core::Utils::loadMeshFromFile(modelFileName, Core::Engine::getInstance().getRenderData());
    primitiveMeshData.animations = {
        Core::Animations::AnimationsUtils::loadAnimationFromFile("assets/mixamo/animations/StandingIdleAnimation.fbx"),
        Core::Animations::AnimationsUtils::loadAnimationFromFile("assets/mixamo/animations/AngryAnimation.fbx"),
        Core::Animations::AnimationsUtils::loadAnimationFromFile("assets/mixamo/animations/BoxingAnimation.fbx"),
        Core::Animations::AnimationsUtils::loadAnimationFromFile("assets/mixamo/animations/HipHopDancingAnimation.fbx"),
        Core::Animations::AnimationsUtils::loadAnimationFromFile("assets/mixamo/animations/StandingReactDeathBackwardAnimation.fbx")
    };
    if (!Core::Renderer::MeshPipelineLayout::init(Core::Engine::getInstance().getRenderData(), Core::Engine::getInstance().getRenderData().rdMeshPipelineLayout))
    {
        Logger::log(1, "%s error: could not init mesh pipeline layout\n", __FUNCTION__);
        return false;
    }

    const std::string vertexShaderFile = "shaders/primitive.vert.spv";
    const std::string fragmentShaderFile = "shaders/primitive.frag.spv";
    if (!MeshPipeline::init(Core::Engine::getInstance().getRenderData(), Core::Engine::getInstance().getRenderData().rdMeshPipelineLayout, Core::Engine::getInstance().getRenderData().rdMeshPipeline,
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
    testMeshOne->getTransform().position = {1, 0, 0};

    auto testMeshTwo = std::make_shared<Mesh>("TestMesh_2", primitiveMeshData.skeleton);
    testMeshTwo->setupAnimations(primitiveMeshData.animations);
    testMeshTwo->initDebugSkeleton(Core::Engine::getInstance().getRenderData());
    testMeshTwo->getTransform().position = {-1, 0, 0};

    Core::Engine::getInstance().getSystem<Scene::Scene>()->addObject(testMeshOne);
    Core::Engine::getInstance().getSystem<Scene::Scene>()->addObject(testMeshTwo);

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
        testMeshOne->addPrimitive(primitive.vertices, primitive.indices, primitiveTexture, Core::Engine::getInstance().getRenderData(), primitive.material,
                            primitive.bones);
        testMeshTwo->addPrimitive(primitive.vertices, primitive.indices, primitiveTexture, Core::Engine::getInstance().getRenderData(), primitive.material,
                                  primitive.bones);
    }

    return true;
}

bool Core::Renderer::VkRenderer::createDebugSkeletonPipelineLayout()
{
    if (!Core::Renderer::DebugSkeletonPipelineLayout::init(Core::Engine::getInstance().getRenderData(), Core::Engine::getInstance().getRenderData().rdDebugSkeletonPipelineLayout))
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
    if (!DebugSkeletonPipeline::init(Core::Engine::getInstance().getRenderData(), Core::Engine::getInstance().getRenderData().rdDebugSkeletonPipelineLayout, Core::Engine::getInstance().getRenderData().rdDebugSkeletonPipeline,
                                     VK_PRIMITIVE_TOPOLOGY_LINE_LIST, vertexShaderFile, fragmentShaderFile))
    {
        Logger::log(1, "%s error: could not init debug skeleton shader pipeline\n", __FUNCTION__);
        return false;
    }

    return true;
}

void Core::Renderer::VkRenderer::handleWindowMoveEvents(int xPosition, int yPosition)
{
    Logger::log(1, "%s: Core::Engine::getInstance().getRenderData().rdWindow has been moved to %i/%i\n", __FUNCTION__, xPosition, yPosition);
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

    Core::Engine::getInstance().getRenderData().rdMoveForward = 0.f;
    if (glfwGetKey(Core::Engine::getInstance().getRenderData().rdWindow, GLFW_KEY_W) == GLFW_PRESS)
    {
        Core::Engine::getInstance().getRenderData().rdMoveForward += 1.f;
    }
    if (glfwGetKey(Core::Engine::getInstance().getRenderData().rdWindow, GLFW_KEY_S) == GLFW_PRESS)
    {
        Core::Engine::getInstance().getRenderData().rdMoveForward -= 1.f;
    }

    Core::Engine::getInstance().getRenderData().rdMoveRight = 0.f;
    if (glfwGetKey(Core::Engine::getInstance().getRenderData().rdWindow, GLFW_KEY_D) == GLFW_PRESS)
    {
        Core::Engine::getInstance().getRenderData().rdMoveRight += 1;
    }
    if (glfwGetKey(Core::Engine::getInstance().getRenderData().rdWindow, GLFW_KEY_A) == GLFW_PRESS)
    {
        Core::Engine::getInstance().getRenderData().rdMoveRight -= 1.f;
    }

    Core::Engine::getInstance().getRenderData().rdMoveUp = 0.f;
    if (glfwGetKey(Core::Engine::getInstance().getRenderData().rdWindow, GLFW_KEY_E) == GLFW_PRESS)
    {
        Core::Engine::getInstance().getRenderData().rdMoveUp += 1.f;
    }
    if (glfwGetKey(Core::Engine::getInstance().getRenderData().rdWindow, GLFW_KEY_Q) == GLFW_PRESS)
    {
        Core::Engine::getInstance().getRenderData().rdMoveUp -= 1.f;
    }
}
