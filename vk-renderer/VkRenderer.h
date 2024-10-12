#pragma once

#include <vulkan/vulkan.h>
#include <GLFW/glfw3.h>
#include "VkRenderData.h"

class VkRenderer
{
  public:
    VkRenderer(GLFWwindow* inWindow);

    bool init(unsigned int width, unsigned int height);

    void setSize(unsigned int width, unsigned int height);

    bool uploadData(VkMesh vertexData);

    bool draw();

    void cleanup();

  private:
    GLFWwindow* mWindow = nullptr;

    int mTriangleCount = 0;

    VkRenderData mRenderData{};

    VkBuffer mVertexBuffer{};

    VkSurfaceKHR mSurface = VK_NULL_HANDLE;

    vkb::PhysicalDevice mPhysicalDevice;

    VmaAllocation mVertexBufferAlloc{};

    bool deviceInit();

    bool getQueue();

    bool createDepthBuffer();

    bool createSwapchain();

    bool recreateSwapchain();

    bool createRenderPass();

    bool createPipeline();

    bool createFramebuffer();

    bool createCommandPool();

    bool createCommandBuffer();

    bool createSyncObjects();

    bool loadTexture();

    bool initVma();
};
