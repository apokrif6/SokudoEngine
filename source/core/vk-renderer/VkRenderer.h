#pragma once

#include <vulkan/vulkan.h>
#include <GLFW/glfw3.h>
#include "VkRenderData.h"
#include "core/ui/UserInterface.h"
#include "core/tools/Timer.h"
#include "core/tools/Camera.h"
#include "core/model/Model.h"
#include "core/model/CoordinateArrowModel.h"
#include "core/events/EventListener.h"
#include "core/model/ArrowModel.h"
#include "core/events/EventDispatcher.h"
#include "core/model/GridModel.h"
#include "Primitive.h"
#include "Mesh.h"
#include "core/scene/Scene.h"
#include <glm/detail/type_quat.hpp>

namespace Core::Renderer
{
class VkRenderer final : public EventListener
{
  public:
    explicit VkRenderer(GLFWwindow* inWindow);

    bool init(unsigned int width, unsigned int height);

    void setSize(unsigned int width, unsigned int height);

    void subscribeToInputEvents(EventDispatcher& eventDispatcher);

    void onEvent(const Event& event) override;

    void beginUploadFrame(VkRenderData& renderData);

    void update(VkRenderData& renderData, float deltaTime);

    void endUploadFrame(VkRenderData& renderData);

    void beginRenderFrame(VkRenderData& renderData);

    bool draw(VkRenderData& renderData);

    void endRenderFrame(VkRenderData& renderData);

    void cleanup(VkRenderData& renderData);

  private:
    Core::Renderer::VkPrimitiveRenderData mPrimitiveRenderData{};

    Core::Model::GridModel mGridModel{};
    Core::Renderer::VkMesh mGridMesh{};

    Core::Model::CoordinateArrowModel mCoordinateArrowsModel{};
    Core::Renderer::VkMesh mCoordinateArrowsMesh{};
    Core::Renderer::VkMesh mEulerCoordinateArrowsMesh{};

    Core::Model::ArrowModel mArrowModel{};
    Core::Renderer::VkMesh mQuaternionArrowMesh{};

    std::unique_ptr<Core::Model::Model> mModel = nullptr;
    std::unique_ptr<Core::Renderer::VkMesh> mEulerModelMesh = nullptr;
    std::unique_ptr<Core::Renderer::VkMesh> mQuaternionModelMesh = nullptr;
    std::unique_ptr<Core::Renderer::VkMesh> mAllMeshes = nullptr;
    unsigned int mLineIndexCount = 0;

    std::shared_ptr<Core::Renderer::VkMesh> mSkeletonMesh = nullptr;
    unsigned int mSkeletonLineIndexCount = 0;

    glm::mat4 mRotYMat = glm::mat4(1.0f);
    glm::mat4 mRotZMat = glm::mat4(1.0f);

    glm::vec3 mEulerModelDist = glm::vec3(-2.5f, 0.f, 0.f);
    glm::vec3 mQuaternionModelDist = glm::vec3(2.5f, 0.f, 0.f);

    glm::vec3 mRotXAxis = glm::vec3(1.0f, 0.0f, 0.0f);
    glm::vec3 mRotYAxis = glm::vec3(0.0f, 1.0f, 0.0f);
    glm::vec3 mRotZAxis = glm::vec3(0.0f, 0.0f, 1.0f);

    glm::mat3 mEulerRotMatrix = glm::mat3(1.0f);
    glm::quat mQuaternionModelOrientation = glm::quat();
    glm::quat mQuaternionModelOrientationConjugate = glm::quat();

    Timer mUploadToVBOTimer{};
    Timer mUploadToUBOTimer{};
    Timer mMatrixGenerateTimer{};

    VkSurfaceKHR mSurface = VK_NULL_HANDLE;

    VkDeviceSize mMinUniformBufferOffsetAlignment = 0;

    std::vector<glm::mat4> mPerspectiveViewMatrices{};

    unsigned int VertexBufferSize = 2000;

#pragma region Camera
    Camera mCamera{};

    void handleCameraMovementKeys();
#pragma endregion Camera

#pragma region VulkanCore
    bool deviceInit();

    bool getQueue();

    bool createSwapchain();

    bool createDepthBuffer();

    bool recreateSwapchain();

    bool createUBO();

    bool createVBO();

    bool createRenderPass();

    bool createPipelineLayout();

    bool createBasicPipeline();

    bool createLinePipeline();

    bool createGridPipeline();

    bool createFramebuffer();

    bool createCommandPool();

    bool createCommandBuffer();

    bool createSyncObjects();

    bool loadTexture();

    bool initVma();

    bool loadMeshWithAssimp();

    bool createDebugSkeletonPipelineLayout();

    bool createDebugSkeletonPipeline();

#pragma endregion Renderer

#pragma region HandleGLFWEvents
  public:
    void handleWindowMoveEvents(int xPosition, int yPosition);

    void handleWindowMinimizedEvents(int minimized);

    void handleWindowMaximizedEvents(int maximized);

    void handleWindowCloseEvents();

    void handleMouseEnterLeaveEvents(int enter);
#pragma endregion HandleGLFWEvents
};
}