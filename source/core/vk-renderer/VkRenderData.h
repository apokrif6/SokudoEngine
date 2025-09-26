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
    std::string rdName;
};

struct VkIndexBufferData
{
    size_t rdIndexBufferSize = 0;
    VkBuffer rdIndexBuffer = VK_NULL_HANDLE;
    VmaAllocation rdIndexBufferAlloc = nullptr;
    VkBuffer rdStagingBuffer = VK_NULL_HANDLE;
    VmaAllocation rdStagingBufferAlloc = nullptr;
    std::string rdName;
};

struct VkUniformBufferData
{
    size_t rdUniformBufferSize = 0;
    VkBuffer rdUniformBuffer = VK_NULL_HANDLE;
    VmaAllocation rdUniformBufferAlloc = nullptr;
    VkDescriptorPool rdUBODescriptorPool = VK_NULL_HANDLE;
    VkDescriptorSetLayout rdUBODescriptorLayout = VK_NULL_HANDLE;
    VkDescriptorSet rdUBODescriptorSet = VK_NULL_HANDLE;
    std::string rdName;
};

struct VkShaderStorageBufferData
{
    size_t rdShaderStorageBufferSize = 0;
    VkBuffer rdShaderStorageBuffer = VK_NULL_HANDLE;
    VmaAllocation rdShaderStorageBufferAlloc = nullptr;
    VkDescriptorPool rdSSBODescriptorPool = VK_NULL_HANDLE;
    VkDescriptorSetLayout rdSSBODescriptorLayout = VK_NULL_HANDLE;
    VkDescriptorSet rdSSBODescriptorSet = VK_NULL_HANDLE;
    std::string rdName;
};

struct VkPrimitiveRenderData
{
    VkVertexBufferData rdModelVertexBufferData{};
    VkIndexBufferData rdModelIndexBufferData{};
    VkTextureData rdModelTexture{};
};

struct VkCubemapData
{
    VkImage image = VK_NULL_HANDLE;
    VkImageView imageView = VK_NULL_HANDLE;
    VmaAllocation imageAlloc = nullptr;
    VkSampler sampler = VK_NULL_HANDLE;

    VkDescriptorPool descriptorPool = VK_NULL_HANDLE;
    VkDescriptorSetLayout descriptorSetLayout = VK_NULL_HANDLE;
    VkDescriptorSet descriptorSet = VK_NULL_HANDLE;

    int widt = 0;
    int height = 0;
};

struct VkRenderData
{
    GLFWwindow* rdWindow = nullptr;

    int rdWidth = 0;
    int rdHeight = 0;

    unsigned int rdTriangleCount = 0;

    int rdFieldOfView = 90;

#pragma region Profiling
    float rdFrameTime = 0.f;
    float rdMatrixGenerateTime = 0.f;
    float rdUploadToVBOTime = 0.f;
    float rdUploadToUBOTime = 0.f;
    float rdAnimationBonesTransformCalculationTime = 0.f;
    float rdUpdateSceneProfilingTime = 0.f;
#pragma endregion

    float rdViewYaw = 0.f;
    float rdViewPitch = 0.f;

    glm::vec3 rdCameraWorldPosition = glm::vec3(0.f, 3.f, 4.5f);

    float rdMoveForward = 0;
    float rdMoveRight = 0;
    float rdMoveUp = 0;

    bool freeCameraMovement = false;

    float rdTickDiff = 0.f;

    VmaAllocator rdAllocator;

    uint32_t rdCurrentImageIndex = 0;

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
    VkDescriptorSetLayout rdMeshModelDescriptorLayout = VK_NULL_HANDLE;

    VkRenderPass rdRenderpass = VK_NULL_HANDLE;
    VkPipelineLayout rdPipelineLayout = VK_NULL_HANDLE;
    VkPipelineLayout rdMeshPipelineLayout = VK_NULL_HANDLE;
    VkPipelineLayout rdDebugSkeletonPipelineLayout = VK_NULL_HANDLE;
    VkPipelineLayout rdSkyboxPipelineLayout = VK_NULL_HANDLE;
    VkPipeline rdBasicPipeline = VK_NULL_HANDLE;
    VkPipeline rdLinePipeline = VK_NULL_HANDLE;
    VkPipeline rdGridPipeline = VK_NULL_HANDLE;
    VkPipeline rdMeshPipeline = VK_NULL_HANDLE;
    VkPipeline rdDebugSkeletonPipeline = VK_NULL_HANDLE;
    VkPipeline rdSkyboxPipeline = VK_NULL_HANDLE;

    VkCommandPool rdCommandPool = VK_NULL_HANDLE;
    VkCommandBuffer rdCommandBuffer = VK_NULL_HANDLE;

    VkSemaphore rdPresentSemaphore = VK_NULL_HANDLE;
    VkSemaphore rdRenderSemaphore = VK_NULL_HANDLE;
    VkFence rdRenderFence = VK_NULL_HANDLE;

    VkTextureData rdPlaceholderTexture{};

    VkVertexBufferData rdVertexBufferData{};

    VkUniformBufferData rdPerspectiveViewMatrixUBO{};

    VkCubemapData rdSkyboxData{};

    VkDescriptorPool rdImguiDescriptorPool = VK_NULL_HANDLE;
};
} // namespace Core::Renderer