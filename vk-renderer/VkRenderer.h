#pragma once

#include <vulkan/vulkan.h>
#include <GLFW/glfw3.h>
#include "VkRenderData.h"
#include "UserInterface.h"
#include "Timer.h"

class VkRenderer
{
  public:
    explicit VkRenderer(GLFWwindow* inWindow);

    bool init(unsigned int width, unsigned int height);

    void setSize(unsigned int width, unsigned int height);

    bool uploadData(VkMesh vertexData);

    bool draw();

    void cleanup();

  private:
    VkRenderData mRenderData{};

    UserInterface mUserInterface{};

    Timer mFrameTimer{};
    Timer mUIGenerateTimer{};
    Timer mUIDrawTimer{};
    Timer mUploadToUBOTimer{};
    Timer mMatrixGenerateTimer{};

    VkBuffer mVertexBuffer{};

    VkSurfaceKHR mSurface = VK_NULL_HANDLE;

    vkb::PhysicalDevice mPhysicalDevice;

    VkDeviceSize mMinUniformBufferOffsetAlignment = 0;

    VmaAllocation mVertexBufferAlloc{};

    VkUploadMatrices mMatrices{};

    bool mShouldUseChangedShader = false;

#pragma region VulkanCore
    bool deviceInit();

    bool getQueue();

    bool createSwapchain();

    bool createDepthBuffer();

    bool recreateSwapchain();

    bool createUBO();

    bool createRenderPass();

    bool createPipelineLayout();

    bool createBasicPipeline();

    bool createChangedPipeline();

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
