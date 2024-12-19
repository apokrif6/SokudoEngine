#pragma once

#include <vulkan/vulkan.h>
#include <GLFW/glfw3.h>
#include "VkRenderData.h"
#include "UserInterface.h"
#include "tools/Timer.h"
#include "tools/Camera.h"
#include "model/Model.h"
#include "CoordinateArrowModel.h"
#include "events/EventListener.h"
#include "ArrowModel.h"
#include "events/EventDispatcher.h"
#include "GridModel.h"
#include "GltfModel.h"
#include <glm/detail/type_quat.hpp>

class VkRenderer : public EventListener
{
  public:
    explicit VkRenderer(GLFWwindow* inWindow);

    bool init(unsigned int width, unsigned int height);

    void setSize(unsigned int width, unsigned int height);

    void subscribeToInputEvents(EventDispatcher& eventDispatcher);

    void onEvent(const Event& event) override;

    bool draw();

    void cleanup();

  private:
    VkRenderData mRenderData{};
    VkGltfRenderData mGltfRenderData{};

    UserInterface mUserInterface{};

    GridModel mGridModel{};
    VkMesh mGridMesh{};

    CoordinateArrowModel mCoordinateArrowsModel{};
    VkMesh mCoordinateArrowsMesh{};
    VkMesh mEulerCoordinateArrowsMesh{};

    ArrowModel mArrowModel{};
    VkMesh mQuaternionArrowMesh{};

    std::unique_ptr<Model> mModel = nullptr;
    std::unique_ptr<VkMesh> mEulerModelMesh = nullptr;
    std::unique_ptr<VkMesh> mQuaternionModelMesh = nullptr;
    std::unique_ptr<VkMesh> mAllMeshes = nullptr;
    unsigned int mLineIndexCount = 0;

    std::shared_ptr<GltfModel> mGltfModel = nullptr;

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

    Timer mFrameTimer{};
    Timer mUIGenerateTimer{};
    Timer mUIDrawTimer{};
    Timer mUploadToVBOTimer{};
    Timer mUploadToUBOTimer{};
    Timer mMatrixGenerateTimer{};

    VkSurfaceKHR mSurface = VK_NULL_HANDLE;

    VkDeviceSize mMinUniformBufferOffsetAlignment = 0;

    VkUploadMatrices mMatrices{};

    unsigned int VertexBufferSize = 2000;

#pragma region Camera
    Camera mCamera{};

    double mLastTickTime = 0.0;

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

    bool createGltfPipelineLayout();

    bool createGltfPipeline();

    bool createFramebuffer();

    bool createCommandPool();

    bool createCommandBuffer();

    bool createSyncObjects();

    bool loadTexture();

    bool initVma();

    bool initUserInterface();

    bool loadGltfModel();

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
