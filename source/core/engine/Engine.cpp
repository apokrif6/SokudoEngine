#include "Engine.h"
#include "core/window/Window.h"

void Core::Engine::init()
{
    std::unique_ptr<Core::Application::Window> ApplicationWindow = std::make_unique<Core::Application::Window>();

    if (mRenderer = ApplicationWindow->init(1280, 900, "Sokudo Engine"); !mRenderer)
    {
        Logger::log(1, "%s error: window init error\n", __FUNCTION__);
        return;
    }

    if (!mUserInterface->init(mRenderData))
    {
        Logger::log(1, "%s error: could not init ImGui\n", __FUNCTION__);
        return;
    }

    ApplicationWindow->mainLoop();
    ApplicationWindow->cleanup();
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

    mRenderer->beginUploadFrame(mRenderData);

    mRenderer->update(mRenderData, mRenderData.rdTickDiff);
    mAnimator->update(mRenderData);
    mScene->update(mRenderData, mRenderData.rdTickDiff);
    mUserInterface->update(mRenderData);

    mRenderer->endUploadFrame(mRenderData);

    mLastTickTime = tickTime;
}

void Core::Engine::draw()
{
    mRenderer->beginRenderFrame(mRenderData);

    mRenderer->draw(mRenderData);
    mScene->draw(mRenderData);
    mUserInterface->draw(mRenderData);

    mRenderer->endRenderFrame(mRenderData);
}

void Core::Engine::cleanup()
{
    vkDeviceWaitIdle(mRenderData.rdVkbDevice);

    mScene->cleanup(mRenderData);
    mUserInterface->cleanup(mRenderData);
    mRenderer->cleanup(mRenderData);
}
