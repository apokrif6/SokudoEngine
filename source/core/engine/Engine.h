#pragma once

#include <memory>
#include "core/scene/Scene.h"
#include "core/vk-renderer/VkRenderer.h"
#include "core/animations/Animator.h"

namespace Core
{
class Engine
{
  public:
    static Engine& getInstance()
    {
        static Engine instance;
        return instance;
    }

    void init();
    void update();
    void draw();
    void cleanup();

    template<typename T>
    T* getSystem()
    {
        // TODO
        // should be replaced with a more generic solution when vkCommand buffer will be handled
        if constexpr (std::is_same_v<T, Scene::Scene>)
        {
            return mScene.get();
        }
        else if constexpr (std::is_same_v<T, Renderer::VkRenderer>)
        {
            return mRenderer.get();
        }
        else if constexpr (std::is_same_v<T, Animations::Animator>)
        {
            return mAnimator.get();
        }
        else
        {
            static_assert(false, "Unsupported system type");
        }
    }

    Core::Renderer::VkRenderData& getRenderData()
    {
        return mRenderData;
    }

  private:
    Engine() = default;
    ~Engine() = default;

    float mLastTickTime = 0.0;
    Timer mFrameTimer{};

    Core::Renderer::VkRenderData mRenderData;

    std::unique_ptr<Renderer::VkRenderer> mRenderer;
    std::unique_ptr<Scene::Scene> mScene = std::make_unique<Scene::Scene>();
    std::unique_ptr<Animations::Animator> mAnimator = std::make_unique<Animations::Animator>();
    std::unique_ptr<Renderer::UserInterface> mUserInterface = std::make_unique<Renderer::UserInterface>();
};
}