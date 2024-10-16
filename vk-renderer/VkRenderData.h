#pragma once

#include "vk_mem_alloc.h"
#include "VkBootstrap.h"

#include <glm/glm.hpp>
#include <GLFW/glfw3.h>

struct VkVertex
{
    glm::vec3 position;
    glm::vec3 color;
    glm::vec2 uv;
};

struct VkMesh
{
    std::vector<VkVertex> vertices;
};

struct VkUploadMatrices
{
    glm::mat4 viewMatrix;
    glm::mat4 projectionMatrix;
};

struct VkRenderData
{
    GLFWwindow* rdWindow = nullptr;

    int rdWidth = 0;
    int rdHeight = 0;

    unsigned int rdTriangleCount = 0;

    float rdFrameTime = 0.0f;

    float rdMillisecondsPerFrameTime = 0.0f;

    VmaAllocator rdAllocator;
    vkb::Instance rdVkbInstance{};
    vkb::Device rdVkbDevice{};
    vkb::Swapchain rdVkbSwapchain{};

    std::vector<VkImage> rdSwapchainImages;
    std::vector<VkImageView> rdSwapchainImageViews;
    std::vector<VkFramebuffer> rdFramebuffers;

    VkQueue rdGraphicsQueue = VK_NULL_HANDLE;
    VkQueue rdPresentQueue = VK_NULL_HANDLE;

    VkImage rdDepthImage = VK_NULL_HANDLE;
    VkImageView rdDepthImageView = VK_NULL_HANDLE;
    VkFormat rdDepthFormat;
    VmaAllocation rdDepthImageAlloc = VK_NULL_HANDLE;

    VkRenderPass rdRenderpass = VK_NULL_HANDLE;
    VkPipelineLayout rdPipelineLayout = VK_NULL_HANDLE;
    VkPipeline rdBasicPipeline = VK_NULL_HANDLE;
    VkPipeline rdChangedPipeline = VK_NULL_HANDLE;

    VkCommandPool rdCommandPool = VK_NULL_HANDLE;
    VkCommandBuffer rdCommandBuffer = VK_NULL_HANDLE;

    VkSemaphore rdPresentSemaphore = VK_NULL_HANDLE;
    VkSemaphore rdRenderSemaphore = VK_NULL_HANDLE;
    VkFence rdRenderFence = VK_NULL_HANDLE;

    VkImage rdTextureImage = VK_NULL_HANDLE;
    VkImageView rdTextureImageView = VK_NULL_HANDLE;
    VkSampler rdTextureSampler = VK_NULL_HANDLE;
    VmaAllocation rdTextureImageAlloc = VK_NULL_HANDLE;

    VkDescriptorPool rdTextureDescriptorPool = VK_NULL_HANDLE;
    VkDescriptorSetLayout rdTextureDescriptorLayout = VK_NULL_HANDLE;
    VkDescriptorSet rdTextureDescriptorSet = VK_NULL_HANDLE;

    VkBuffer rdUBOBuffer = VK_NULL_HANDLE;
    VmaAllocation rdUBOBufferAlloc = nullptr;

    VkDescriptorPool rdUBODescriptorPool = VK_NULL_HANDLE;
    VkDescriptorSetLayout rdUBODescriptorLayout = VK_NULL_HANDLE;
    VkDescriptorSet rdUBODescriptorSet = VK_NULL_HANDLE;

    VkDescriptorPool rdImguiDescriptorPool = VK_NULL_HANDLE;
};
