#include "Engine.h"
#include "core/window/Window.h"

void Core::Engine::init()
{
    Core::Application::Window* applicationWindow = new Core::Application::Window();
    applicationWindow->init(1280, 900, "Sokudo Engine");

    auto* renderer = createSystem<Renderer::VkRenderer>(applicationWindow->getGLFWwindow());
    if (!renderer->init(1280, 900))
    {
        Logger::log(1, "%s error: could not init renderer", __FUNCTION__);
        return;
    }

    auto* userInterface = createSystem<Renderer::UserInterface>();
    if (!userInterface->init(mRenderData))
    {
        Logger::log(1, "%s error: could not init ImGui", __FUNCTION__);
        return;
    }

    createSystem<Scene::Scene>();
    createSystem<Animations::Animator>();

    // Sandbox
    if (!renderer->loadMeshWithAssimp())
    {
        Logger::log(1, "%s error: could not load mesh with assimp", __FUNCTION__);
        return;
    }

    applicationWindow->mainLoop();
    applicationWindow->cleanup();
}

void Core::Engine::update()
{
    if (mPaused)
    {
        return;
    }

    auto tickTime = static_cast<float>(glfwGetTime());
    mRenderData.rdTickDiff = tickTime - mLastTickTime;

    mRenderData.rdFrameTime = mFrameTimer.stop();
    mFrameTimer.start();

    getSystem<Core::Renderer::VkRenderer>()->beginUploadFrame(mRenderData);

    for (auto* updatable : mUpdatables)
    {
        updatable->update(mRenderData, mRenderData.rdTickDiff);
    }

    getSystem<Core::Renderer::VkRenderer>()->endUploadFrame(mRenderData);

    mLastTickTime = tickTime;
}

void Core::Engine::draw()
{
    getSystem<Core::Renderer::VkRenderer>()->beginRenderFrame(mRenderData);

    getSystem<Core::Renderer::VkRenderer>()->beginOffscreenRenderPass(mRenderData);

    for (auto* drawable : mDrawables)
    {
        if (drawable->getDrawLayer() == System::DrawLayer::World)
        {
            drawable->draw(mRenderData);
        }
    }

    getSystem<Core::Renderer::VkRenderer>()->endOffscreenRenderPass(mRenderData);

    getSystem<Core::Renderer::VkRenderer>()->beginFinalRenderPass(mRenderData);

    for (auto* drawable : mDrawables)
    {
        if (drawable->getDrawLayer() == System::DrawLayer::Overlay)
        {
            drawable->draw(mRenderData);
        }
    }

    getSystem<Core::Renderer::VkRenderer>()->endFinalRenderPass(mRenderData);

    getSystem<Core::Renderer::VkRenderer>()->endRenderFrame(mRenderData);
}

void Core::Engine::cleanup()
{
    vkDeviceWaitIdle(mRenderData.rdVkbDevice);

    getSystem<Core::Scene::Scene>()->cleanup(mRenderData);
    getSystem<Core::Renderer::UserInterface>()->cleanup(mRenderData);
    getSystem<Core::Renderer::VkRenderer>()->cleanup(mRenderData);
}
