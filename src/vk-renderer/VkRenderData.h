#pragma once

#include "vk_mem_alloc.h"
#include "VkBootstrap.h"

#include <glm/glm.hpp>
#include <GLFW/glfw3.h>
#include <array>

struct VkVertex
{
    glm::vec3 position;
    glm::vec3 color;
    glm::vec2 uv;
    //glm::vec3 nearPoint;
    // glm::vec3 farPoint;

    static VkVertexInputBindingDescription getBindingDescription()
    {
        VkVertexInputBindingDescription bindingDescription{};
        bindingDescription.binding = 0;
        bindingDescription.stride = sizeof(VkVertex);
        bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

        return bindingDescription;
    }

    static std::array<VkVertexInputAttributeDescription, 3> getAttributeDescriptions() {
        std::array<VkVertexInputAttributeDescription, 3> attributeDescriptions{};
        attributeDescriptions[0].binding = 0;
        attributeDescriptions[0].location = 0;
        attributeDescriptions[0].format = VK_FORMAT_R32G32B32_SFLOAT;
        attributeDescriptions[0].offset = offsetof(VkVertex, position);

        attributeDescriptions[1].binding = 0;
        attributeDescriptions[1].location = 1;
        attributeDescriptions[1].format = VK_FORMAT_R32G32B32_SFLOAT;
        attributeDescriptions[1].offset = offsetof(VkVertex, color);

        attributeDescriptions[2].binding = 0;
        attributeDescriptions[2].location = 2;
        attributeDescriptions[2].format = VK_FORMAT_R32G32_SFLOAT;
        attributeDescriptions[2].offset = offsetof(VkVertex, uv);

        return attributeDescriptions;
    }
};

struct VkMesh
{
    std::vector<VkVertex> vertices;
};

struct VkUploadMatrices
{
    glm::mat4 viewMatrix;
    glm::mat4 projectionMatrix;
    glm::vec3 position;
};

struct VkRenderData
{
    GLFWwindow* rdWindow = nullptr;

    int rdWidth = 0;
    int rdHeight = 0;

    unsigned int rdTriangleCount = 0;

    int rdFieldOfView = 90;

    float rdFrameTime = 0.0f;
    float rdMatrixGenerateTime = 0.0f;
    float rdUploadToVBOTime = 0.0f;
    float rdUploadToUBOTime = 0.0f;
    float rdUIGenerateTime = 0.0f;
    float rdUIDrawTime = 0.0f;

    float rdViewYaw = 180.f;
    float rdViewPitch = 0.f;

    glm::vec3 rdCameraWorldPosition = glm::vec3(0.0f, 0.0f, -0.5f);

    bool rdDrawWorldCoordinateArrows = true;
    bool rdDrawModelCoordinateArrows = true;
    bool rdResetAngles = false;

    int rdRotXAngle = 0;
    int rdRotYAngle = 0;
    int rdRotZAngle = 0;

    float rdMoveForward = 0;
    float rdMoveRight = 0;
    float rdMoveUp = 0;

    float rdTickDiff = 0.f;

    VmaAllocator rdAllocator;

    vkb::Instance rdVkbInstance{};
    vkb::PhysicalDevice rdVkbPhysicalDevice{};
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
    VkPipeline rdLinePipeline = VK_NULL_HANDLE;
    VkPipeline rdGridPipeline = VK_NULL_HANDLE;

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

    unsigned int rdVertexBufferSize = 2048;
    VkBuffer rdVertexBuffer = VK_NULL_HANDLE;
    VmaAllocation rdVertexBufferAlloc = nullptr;
    VkBuffer rdVertexStagingBuffer = VK_NULL_HANDLE;
    VmaAllocation rdVertexStagingBufferAlloc = nullptr;

    VkBuffer rdUBOBuffer = VK_NULL_HANDLE;
    VmaAllocation rdUBOBufferAlloc = nullptr;

    VkDescriptorPool rdUBODescriptorPool = VK_NULL_HANDLE;
    VkDescriptorSetLayout rdUBODescriptorLayout = VK_NULL_HANDLE;
    VkDescriptorSet rdUBODescriptorSet = VK_NULL_HANDLE;

    VkDescriptorPool rdImguiDescriptorPool = VK_NULL_HANDLE;
};
