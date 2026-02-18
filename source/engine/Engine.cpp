#include "Engine.h"

#include "core/components/ComponentFactory.h"
#include "ui/UserInterface.h"
#include "window/Window.h"

void Core::Engine::init()
{
    Application::Window* applicationWindow = new Application::Window();
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

    Component::ComponentFactory::registerAll();

    applicationWindow->bindInputs();
    applicationWindow->mainLoop();
    applicationWindow->cleanup();

    delete applicationWindow;
}

void Core::Engine::update()
{
    if (mState == EngineState::Paused || mState == EngineState::Loading)
    {
        return;
    }

    auto tickTime = static_cast<float>(glfwGetTime());
    mRenderData.rdTickDiff = tickTime - mLastTickTime;

    mRenderData.rdFrameTime = mFrameTimer.stop();
    mFrameTimer.start();

    getSystem<Renderer::VkRenderer>()->beginUploadFrame(mRenderData);

    for (auto* updatable : mUpdatables)
    {
        updatable->update(mRenderData, mRenderData.rdTickDiff);
    }

    getSystem<Renderer::VkRenderer>()->endUploadFrame(mRenderData);

    mLastTickTime = tickTime;
}

void Core::Engine::draw()
{
    getSystem<Renderer::VkRenderer>()->beginRenderFrame(mRenderData);

    getSystem<Renderer::VkRenderer>()->beginOffscreenRenderPass(mRenderData);

    for (auto* drawable : mDrawables)
    {
        if (drawable->getDrawLayer() == System::DrawLayer::World)
        {
            drawable->draw(mRenderData);
        }
    }

    getSystem<Renderer::VkRenderer>()->endOffscreenRenderPass(mRenderData);

    getSystem<Renderer::VkRenderer>()->beginFinalRenderPass(mRenderData);

    for (auto* drawable : mDrawables)
    {
        if (drawable->getDrawLayer() == System::DrawLayer::Overlay)
        {
            drawable->draw(mRenderData);
        }
    }

    getSystem<Renderer::VkRenderer>()->endFinalRenderPass(mRenderData);

    getSystem<Renderer::VkRenderer>()->endRenderFrame(mRenderData);
}

void Core::Engine::cleanup()
{
    vkDeviceWaitIdle(mRenderData.rdVkbDevice);

    getSystem<Scene::Scene>()->cleanup(mRenderData);
    getSystem<Renderer::UserInterface>()->cleanup(mRenderData);
    getSystem<Renderer::VkRenderer>()->cleanup(mRenderData);
}
