#pragma once

#include "vk_mem_alloc.h"
#include "VkBootstrap.h"
#include <glm/glm.hpp>
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/compatibility.hpp>
#include <GLFW/glfw3.h>
#include <array>

namespace Core::Renderer
{
struct Vertex
{
    glm::vec3 position{};
    glm::vec3 normal{};
    glm::vec4 tangent{};
    glm::vec4 color{};
    glm::vec2 uv{};
    glm::vec4 weights{};
    glm::ivec4 boneID{};

    static VkVertexInputBindingDescription getBindingDescription()
    {
        VkVertexInputBindingDescription bindingDescription{};
        bindingDescription.binding = 0;
        bindingDescription.stride = sizeof(Vertex);
        bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

        return bindingDescription;
    }
};

struct GlobalSceneData
    {
    glm::mat4 view;
    glm::mat4 projection;
    glm::vec4 camPos;
    glm::vec4 lightPositions[4];
    glm::vec4 lightColors[4];
    glm::ivec4 lightCount;
};

constexpr size_t maxNumberOfBones = 200;
struct PrimitiveData
{
    glm::mat4 model;
    glm::mat4 bones[maxNumberOfBones];
};

constexpr size_t maxNumberOfMaterials = 128;
struct MaterialInfo
{
    glm::vec4 baseColorFactor = glm::vec4(1.f);
    glm::vec4 emissiveFactor = glm::vec4(0.f);
    float metallicFactor = 0.f;
    float roughnessFactor = 1.f;
    int useAlbedoMap = 0;
    int useNormalMap = 0;
    int useMetallicRoughnessMap = 0;
    int useAOMap = 0;
    int useEmissiveMap = 0;

    int padding[2];
};

// dude move this somewhere else
struct CameraInfo
{
    glm::vec4 position;
};

constexpr size_t maxNumberOfLights = 4;
struct LightsInfo
{
    glm::vec4 positions[maxNumberOfLights];
    glm::vec4 colors[maxNumberOfLights];
    glm::ivec4 count;
};

struct alignas(16) CaptureInfo
{
    glm::mat4 views[6];
    glm::mat4 projection;
};

struct alignas(16) PrimitiveFlagsPushConstants
{
    int hasSkinning = 0;
};

struct VkTextureData
{
    std::string name;

    VkImage image = VK_NULL_HANDLE;
    VkImageView imageView = VK_NULL_HANDLE;
    VkSampler sampler = VK_NULL_HANDLE;
    VmaAllocation imageAlloc = nullptr;

    VkDescriptorPool descriptorPool = VK_NULL_HANDLE;
    VkDescriptorSetLayout descriptorSetLayout = VK_NULL_HANDLE;
    VkDescriptorSet descriptorSet = VK_NULL_HANDLE;
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
    bool ownsLayout = true;
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

struct VkPBRMaterialData
{
    VkTextureData albedoTexture{};
    VkTextureData metallicRoughnessTexture{};
    VkTextureData normalTexture{};
    VkTextureData aoTexture{};
    VkTextureData emissiveTexture{};

    int hasAlbedoMap = 0;
    int hasNormalMap = 0;
    int hasMetallicMap = 0;
    int hasRoughnessMap = 0;
    int hasAOMap = 0;
    int hasEmissiveMap = 0;

    MaterialInfo materialInfo;
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
};

struct ViewportData
{
    VkImage image;
    VkImageView imageView;
    VmaAllocation imageAlloc;

    VkRenderPass renderpass;
    VkFramebuffer framebuffer;
    VkDescriptorSet descriptorSet;

    glm::int2 size = glm::int2{0, 0};
};

struct PipelineConfig
{
    VkBool32 useVertexInput = VK_TRUE;

    VkBool32 enableDepthTest = VK_TRUE;
    VkBool32 enableDepthWrite = VK_FALSE;

    VkBool32 enableBlending = VK_FALSE;
    VkBlendOp colorBlendOp = VK_BLEND_OP_ADD;
    VkBlendFactor srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
    VkBlendFactor dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
    VkBlendOp alphaBlendOp = VK_BLEND_OP_ADD;

    VkCompareOp depthCompareOp = VK_COMPARE_OP_LESS_OR_EQUAL;
    VkCullModeFlags cullMode = VK_CULL_MODE_BACK_BIT;
    VkFrontFace frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
};

struct PipelineLayoutConfig
{
    std::vector<VkDescriptorSetLayout> setLayouts;
    std::vector<VkPushConstantRange> pushConstantRanges;
};

struct IBLData
{
    VkDescriptorSetLayout rdSingleCubemapDescriptorLayout = VK_NULL_HANDLE;

    VkRenderPass rdIBLRenderpass = VK_NULL_HANDLE;

    VkCubemapData rdIrradianceMap{};

    VkCubemapData rdPrefilterMap{};

    VkTextureData rdBRDFLUT{};
};

struct VkRenderData
{
    GLFWwindow* rdWindow = nullptr;

    int rdWidth = 0;
    int rdHeight = 0;

    int rdFieldOfView = 90;

#pragma region Profiling
    float rdFrameTime = 0.f;
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

    // TODO
    // where to move this?
#pragma region RenderingFeatures
    bool shouldDrawSkybox = true;
    bool shouldDrawGrid = true;
#pragma endregion

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

#pragma region DescriptorLayouts
    VkDescriptorSetLayout rdGlobalSceneDescriptorLayout = VK_NULL_HANDLE;
    VkDescriptorSetLayout rdPrimitiveDataDescriptorLayout = VK_NULL_HANDLE;
    VkDescriptorSetLayout rdPrimitiveTextureDescriptorLayout = VK_NULL_HANDLE;
    VkDescriptorSetLayout rdPrimitiveMaterialDescriptorLayout = VK_NULL_HANDLE;
#pragma endregion

    VkRenderPass rdRenderpass = VK_NULL_HANDLE;
    VkPipelineLayout rdPipelineLayout = VK_NULL_HANDLE;
    VkPipelineLayout rdMeshPipelineLayout = VK_NULL_HANDLE;
    VkPipelineLayout rdDebugSkeletonPipelineLayout = VK_NULL_HANDLE;
    VkPipelineLayout rdSkyboxPipelineLayout = VK_NULL_HANDLE;
    VkPipelineLayout rdHDRToCubemapPipelineLayout = VK_NULL_HANDLE;
    VkPipelineLayout rdIrradiancePipelineLayout = VK_NULL_HANDLE;
    VkPipelineLayout rdPrefilterPipelineLayout = VK_NULL_HANDLE;
    VkPipelineLayout rdBRDFLUTPipelineLayout = VK_NULL_HANDLE;
    VkPipeline rdGridPipeline = VK_NULL_HANDLE;
    VkPipeline rdMeshPipeline = VK_NULL_HANDLE;
    VkPipeline rdDebugSkeletonPipeline = VK_NULL_HANDLE;
    VkPipeline rdSkyboxPipeline = VK_NULL_HANDLE;
    VkPipeline rdHDRToCubemapPipeline = VK_NULL_HANDLE;
    VkPipeline rdIrradiancePipeline = VK_NULL_HANDLE;
    VkPipeline rdPrefilterPipeline = VK_NULL_HANDLE;
    VkPipeline rdBRDFLUTPipeline = VK_NULL_HANDLE;

    VkCommandPool rdCommandPool = VK_NULL_HANDLE;
    VkCommandBuffer rdCommandBuffer = VK_NULL_HANDLE;

    VkRenderPass rdHDRToCubemapRenderpass = VK_NULL_HANDLE;

    VkSemaphore rdPresentSemaphore = VK_NULL_HANDLE;
    VkSemaphore rdRenderSemaphore = VK_NULL_HANDLE;
    VkFence rdRenderFence = VK_NULL_HANDLE;

    VkTextureData rdPlaceholderTexture{};

    VkVertexBufferData rdVertexBufferData{};

    // stores only GlobalScene data
    VkUniformBufferData rdGlobalSceneUBO{};

    VkUniformBufferData rdCaptureUBO{};

    VkTextureData rdHDRTexture{};

    VkCubemapData rdSkyboxData{};

    IBLData rdIBLData{};

    VkDescriptorPool rdImguiDescriptorPool;

    VkDescriptorPool rdMaterialDescriptorPool;

#pragma region DummyDescriptors
    VkUniformBufferData rdDummyBonesUBO{};
#pragma endregion

#pragma region Viewport
    ViewportData rdViewportTarget{};

    bool rdViewportHovered = false;
#pragma endregion
};
} // namespace Core::Renderer