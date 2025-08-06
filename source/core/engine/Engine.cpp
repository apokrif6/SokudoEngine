#include "Engine.h"
#include "core/window/Window.h"

void Core::Engine::init()
{
    std::unique_ptr<Core::Application::Window> ApplicationWindow = std::make_unique<Core::Application::Window>();

    mRenderer = ApplicationWindow->init(1280, 900, "Sokudo Engine");

    if (!mRenderer)
    {
        Logger::log(1, "%s error: window init error\n", __FUNCTION__);
    }


    ApplicationWindow->mainLoop();
    ApplicationWindow->cleanup();
}

void Core::Engine::update()
{
    auto tickTime = static_cast<float>(glfwGetTime());
    mRenderData.rdTickDiff = tickTime - mLastTickTime;

    mRenderData.rdFrameTime = mFrameTimer.stop();
    mFrameTimer.start();

    mRenderer->beginUploadFrame(mRenderData);

    mRenderer->update(mRenderData, mRenderData.rdTickDiff);
    mScene->update(mRenderData, mRenderData.rdTickDiff);

    mRenderer->endUploadFrame(mRenderData);

    mLastTickTime = tickTime;
}

void Core::Engine::draw()
{
    mRenderer->beginRenderFrame(mRenderData);

    mScene->draw(mRenderData);
    mRenderer->draw(mRenderData);

    mRenderer->endRenderFrame(mRenderData);
}

void Core::Engine::cleanup()
{
    mRenderer->cleanup(mRenderData);
}
