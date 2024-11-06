#pragma once

#include <vulkan/vulkan.h>
#include <GLFW/glfw3.h>
#include "VkRenderData.h"
#include "UserInterface.h"
#include "Timer.h"
#include "Camera.h"
#include "Model.h"
#include "CoordinateArrowModel.h"

class VkRenderer
{
  public:
    explicit VkRenderer(GLFWwindow* inWindow);

    bool init(unsigned int width, unsigned int height);

    void setSize(unsigned int width, unsigned int height);

    bool draw();

    void cleanup();

  private:
    VkRenderData mRenderData{};

    UserInterface mUserInterface{};

    CoordinateArrowModel mCoordinateArrowsModel{};
    VkMesh mCoordArrowsMesh{};
    VkMesh mEulerCoordArrowsMesh{};

    std::unique_ptr<Model> mModel = nullptr;
    std::unique_ptr<VkMesh> mEulerModelMesh = nullptr;
    std::unique_ptr<VkMesh> mAllMeshes = nullptr;
    unsigned int mLineIndexCount = 0;

    glm::mat4 mRotYMat = glm::mat4(1.0f);
    glm::mat4 mRotZMat = glm::mat4(1.0f);

    glm::vec3 mRotXAxis = glm::vec3(1.0f, 0.0f, 0.0f);
    glm::vec3 mRotYAxis = glm::vec3(0.0f, 1.0f, 0.0f);
    glm::vec3 mRotZAxis = glm::vec3(0.0f, 0.0f, 1.0f);

    glm::mat3 mEulerRotMatrix = glm::mat3(1.0f);
    glm::vec3 mEulerModelDist = glm::vec3(-2.5f, 0.0f, 0.0f);

    Timer mFrameTimer{};
    Timer mUIGenerateTimer{};
    Timer mUIDrawTimer{};
    Timer mUploadToVBOTimer{};
    Timer mUploadToUBOTimer{};
    Timer mMatrixGenerateTimer{};

    VkBuffer mVertexBuffer{};

    VkSurfaceKHR mSurface = VK_NULL_HANDLE;

    VkDeviceSize mMinUniformBufferOffsetAlignment = 0;

    VmaAllocation mVertexBufferAlloc{};

    VkUploadMatrices mMatrices{};

#pragma region Camera
    Camera mCamera{};

    bool mMouseLock = false;
    int mMouseXPosition = 0;
    int mMouseYPosition = 0;

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

    bool createFramebuffer();

    bool createCommandPool();

    bool createCommandBuffer();

    bool createSyncObjects();

    bool loadTexture();

    bool initVma();

    bool initUserInterface();

#pragma endregion Renderer

#pragma region HandleGLFWEvents
  public:
    void handleWindowMoveEvents(int xPosition, int yPosition);

    void handleWindowMinimizedEvents(int minimized);

    void handleWindowMaximizedEvents(int maximized);

    void handleWindowCloseEvents();

    void handleKeyEvents(int key, int scancode, int action, int mods);

    void handleMouseButtonEvents(int button, int action, int mods);

    void handleMousePositionEvents(double xPosition, double yPosition);

    void handleMouseEnterLeaveEvents(int enter);
#pragma endregion HandleGLFWEvents
};
