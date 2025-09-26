#pragma once

#include <vulkan/vulkan.h>
#include <GLFW/glfw3.h>
#include "VkRenderData.h"
#include "core/ui/UserInterface.h"
#include "core/tools/Timer.h"
#include "core/tools/Camera.h"
#include "core/model/CoordinateArrowModel.h"
#include "core/events/EventListener.h"
#include "core/model/ArrowModel.h"
#include "core/events/EventDispatcher.h"
#include "core/model/GridModel.h"
#include "Primitive.h"
#include "core/scene/objects/Mesh.h"
#include "core/scene/Scene.h"
#include <glm/detail/type_quat.hpp>

namespace Core::Renderer
{
class VkRenderer final : public EventListener
{
  public:
    explicit VkRenderer(GLFWwindow* inWindow);

    bool init(unsigned int width, unsigned int height);

    void handleWindowResizeEvents(int width, int height);

    void onEvent(const Event& event) override;

    void beginUploadFrame(VkRenderData& renderData);

    void update(VkRenderData& renderData, float deltaTime);

    void endUploadFrame(VkRenderData& renderData);

    void beginRenderFrame(VkRenderData& renderData);

    bool draw(VkRenderData& renderData);

    void endRenderFrame(VkRenderData& renderData);

    void cleanup(VkRenderData& renderData);

    const std::vector<glm::mat4>& getPerspectiveViewMatrices()
    {
        return mPerspectiveViewMatrices;
    }

  private:
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

    bool createFramebuffer();

    bool createCommandPool();

    bool createCommandBuffer();

    bool createSyncObjects();

    bool loadPlaceholderTexture();

    bool initVma();

    bool loadMeshWithAssimp();

    bool createDebugSkeletonPipelineLayout();

    bool createDebugSkeletonPipeline();
#pragma endregion Renderer

#pragma region Skybox
    bool createSkyboxPipelineLayout();

    bool createSkyboxPipeline();

    bool loadSkybox();

    void drawSkybox() const;
#pragma endregion Skybox

#pragma region Grid
    bool createGridPipeline();
#pragma endregion Grid

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