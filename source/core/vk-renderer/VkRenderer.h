#pragma once

#include <vulkan/vulkan.h>
#include <GLFW/glfw3.h>
#include "VkRenderData.h"
#include "core/tools/Timer.h"
#include "core/tools/Camera.h"
#include "core/events/EventListener.h"
#include "Primitive.h"
#include "core/scene/objects/Mesh.h"
#include "core/scene/Scene.h"
#include "core/vk-renderer/viewport/ViewportTarget.h"
#include "core/system/System.h"
#include "core/system/Updatable.h"
#include "core/system/Drawable.h"
#include <glm/detail/type_quat.hpp>

namespace Core::Renderer
{
class VkRenderer final : public EventListener,
                         public System::ISystem,
                         public System::IUpdatable,
                         public System::IDrawable
{
public:
    explicit VkRenderer(GLFWwindow* inWindow);

    bool init(unsigned int width, unsigned int height);

    void handleWindowResizeEvents(int width, int height);

    void onEvent(const Event& event) override;

    virtual System::DrawLayer getDrawLayer() const override { return System::DrawLayer::World; }

    // TODO
    // probably should be moved to some RenderSystem, I don't know yet
    virtual void draw(VkRenderData& renderData) override;

    void beginUploadFrame(VkRenderData& renderData);

    virtual void update(VkRenderData& renderData, float deltaTime) override;

    void endUploadFrame(VkRenderData& renderData);

    void beginRenderFrame(VkRenderData& renderData);

    void beginOffscreenRenderPass(VkRenderData& renderData);

    void endOffscreenRenderPass(VkRenderData& renderData);

    void beginFinalRenderPass(VkRenderData& renderData);

    void endFinalRenderPass(VkRenderData& renderData);

    void endRenderFrame(VkRenderData& renderData);

    void cleanup(VkRenderData& renderData);

    const std::vector<glm::mat4>& getPerspectiveViewMatrices() { return mPerspectiveViewMatrices; }

    void resizeViewportTarget(glm::int2 size);

    // TODO
    // move to Sandbox
    bool loadMeshWithAssimp();

    void initCaptureResources();

private:
    Timer mUploadToVBOTimer{};
    Timer mUploadToUBOTimer{};
    Timer mMatrixGenerateTimer{};

    VkSurfaceKHR mSurface = VK_NULL_HANDLE;

    VkDeviceSize mMinUniformBufferOffsetAlignment = 0;

    std::vector<glm::mat4> mPerspectiveViewMatrices{};

    unsigned int VertexBufferSize = 2000;

    std::unique_ptr<ViewportTarget> mViewportTarget =
        std::make_unique<ViewportTarget>();

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

    bool createViewportRenderpass();

    bool createPipelineLayout();

    bool createFramebuffer();

    bool createCommandPool();

    bool createCommandBuffer();

    bool createSyncObjects();

    bool loadPlaceholderTexture();

    bool createDummyBonesTransformUBO();

    bool initVma();

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

    void drawGrid() const;
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
} // namespace Core::Renderer