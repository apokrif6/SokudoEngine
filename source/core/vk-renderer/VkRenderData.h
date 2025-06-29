#pragma once

#include "vk_mem_alloc.h"
#include "VkBootstrap.h"
#include <glm/glm.hpp>
#include <GLFW/glfw3.h>
#include <array>

namespace Core::Renderer
{
struct NewVertex
{
    glm::vec3 position{};
    glm::vec3 normal{};
    glm::vec3 tangent{};
    glm::vec4 color{};
    glm::vec2 uv{};
    float weights[4]{};
    int boneID[4]{};

    static VkVertexInputBindingDescription getBindingDescription()
    {
        VkVertexInputBindingDescription bindingDescription{};
        bindingDescription.binding = 0;
        bindingDescription.stride = sizeof(NewVertex);
        bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

        return bindingDescription;
    }
};

struct MaterialInfo
{
    glm::vec4 baseColorFactor = glm::vec4(1.0f);
    alignas(16) int useTexture = 0;
};

struct VkVertex
{
    glm::vec3 position;
    glm::vec3 color;
    glm::vec2 uv;

    static VkVertexInputBindingDescription getBindingDescription()
    {
        VkVertexInputBindingDescription bindingDescription{};
        bindingDescription.binding = 0;
        bindingDescription.stride = sizeof(VkVertex);
        bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

        return bindingDescription;
    }
};

struct VkMesh
{
    std::vector<VkVertex> vertices;
};

struct VkTextureData
{
    std::string texName;

    VkImage texTextureImage = VK_NULL_HANDLE;
    VkImageView texTextureImageView = VK_NULL_HANDLE;
    VkSampler texTextureSampler = VK_NULL_HANDLE;
    VmaAllocation texTextureImageAlloc = nullptr;

    VkDescriptorPool texTextureDescriptorPool = VK_NULL_HANDLE;
    VkDescriptorSetLayout texTextureDescriptorLayout = VK_NULL_HANDLE;
    VkDescriptorSet texTextureDescriptorSet = VK_NULL_HANDLE;
};

struct VkVertexBufferData
{
    size_t rdVertexBufferSize = 0;
    VkBuffer rdVertexBuffer = VK_NULL_HANDLE;
    VmaAllocation rdVertexBufferAlloc = nullptr;
    VkBuffer rdStagingBuffer = VK_NULL_HANDLE;
    VmaAllocation rdStagingBufferAlloc = nullptr;
};

struct VkIndexBufferData
{
    size_t rdIndexBufferSize = 0;
    VkBuffer rdIndexBuffer = VK_NULL_HANDLE;
    VmaAllocation rdIndexBufferAlloc = nullptr;
    VkBuffer rdStagingBuffer = VK_NULL_HANDLE;
    VmaAllocation rdStagingBufferAlloc = nullptr;
};

struct VkUniformBufferData
{
    size_t rdUniformBufferSize = 0;
    VkBuffer rdUniformBuffer = VK_NULL_HANDLE;
    VmaAllocation rdUniformBufferAlloc = nullptr;

    VkDescriptorPool rdUBODescriptorPool = VK_NULL_HANDLE;
    VkDescriptorSetLayout rdUBODescriptorLayout = VK_NULL_HANDLE;
    VkDescriptorSet rdUBODescriptorSet = VK_NULL_HANDLE;
};

struct VkShaderStorageBufferData
{
    size_t rdShaderStorageBufferSize = 0;
    VkBuffer rdShaderStorageBuffer = VK_NULL_HANDLE;
    VmaAllocation rdShaderStorageBufferAlloc = nullptr;

    VkDescriptorPool rdSSBODescriptorPool = VK_NULL_HANDLE;
    VkDescriptorSetLayout rdSSBODescriptorLayout = VK_NULL_HANDLE;
    VkDescriptorSet rdSSBODescriptorSet = VK_NULL_HANDLE;
};

struct VkPrimitiveRenderData
{
    VkVertexBufferData rdModelVertexBufferData{};
    VkIndexBufferData rdModelIndexBufferData{};
    VkTextureData rdModelTexture{};
};

struct VkRenderData
{
    GLFWwindow* rdWindow = nullptr;

    int rdWidth = 0;
    int rdHeight = 0;

    unsigned int rdTriangleCount = 0;
    unsigned int rdGltfTriangleCount = 0;

    int rdFieldOfView = 90;

    float rdFrameTime = 0.0f;
    float rdMatrixGenerateTime = 0.0f;
    float rdUploadToVBOTime = 0.0f;
    float rdUploadToUBOTime = 0.0f;
    float rdUIGenerateTime = 0.0f;
    float rdUIDrawTime = 0.0f;

    float rdViewYaw = 0.f;
    float rdViewPitch = 0.f;

    glm::vec3 rdCameraWorldPosition = glm::vec3(0.f, 3.f, 4.5f);

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

    bool shouldPlayAnimation = false;
    bool shouldDrawDebugSkeleton = false;
    int selectedAnimationIndexToPlay = 0;

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

    VkDescriptorSetLayout rdMeshTextureDescriptorLayout = VK_NULL_HANDLE;
    VkDescriptorSetLayout rdMeshViewMatrixDescriptorLayout = VK_NULL_HANDLE;
    VkDescriptorSetLayout rdMeshMaterialDescriptorLayout = VK_NULL_HANDLE;
    VkDescriptorSetLayout rdMeshBonesTransformDescriptorLayout = VK_NULL_HANDLE;

    VkRenderPass rdRenderpass = VK_NULL_HANDLE;
    VkPipelineLayout rdPipelineLayout = VK_NULL_HANDLE;
    VkPipelineLayout rdMeshPipelineLayout = VK_NULL_HANDLE;
    VkPipelineLayout rdDebugSkeletonPipelineLayout = VK_NULL_HANDLE;
    VkPipeline rdBasicPipeline = VK_NULL_HANDLE;
    VkPipeline rdLinePipeline = VK_NULL_HANDLE;
    VkPipeline rdGridPipeline = VK_NULL_HANDLE;
    VkPipeline rdMeshPipeline = VK_NULL_HANDLE;
    VkPipeline rdDebugSkeletonPipeline = VK_NULL_HANDLE;

    VkCommandPool rdCommandPool = VK_NULL_HANDLE;
    VkCommandBuffer rdCommandBuffer = VK_NULL_HANDLE;

    VkSemaphore rdPresentSemaphore = VK_NULL_HANDLE;
    VkSemaphore rdRenderSemaphore = VK_NULL_HANDLE;
    VkFence rdRenderFence = VK_NULL_HANDLE;

    VkTextureData rdModelTexture{};

    VkVertexBufferData rdVertexBufferData{};

    VkUniformBufferData rdPerspectiveViewMatrixUBO{};
    VkShaderStorageBufferData rdJointMatrixSSBO{};

    VkDescriptorPool rdImguiDescriptorPool = VK_NULL_HANDLE;
};
} // namespace Core::Renderer