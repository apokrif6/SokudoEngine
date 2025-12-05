#pragma once

#include <memory>
#include <typeindex>
#include "core/scene/Scene.h"
#include "core/vk-renderer/VkRenderer.h"
#include "core/animations/Animator.h"

namespace Core
{
template<class>
inline constexpr bool always_false_v = false;

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

    template<typename T, typename... Args>
    T* createSystem(Args&&... args)
    {
        static_assert(std::is_base_of_v<System::ISystem, T>, "T must derive from ISystem");

        auto ptr = std::make_unique<T>(std::forward<Args>(args)...);
        T* raw = ptr.get();

        mSystems[typeid(T)] = std::move(ptr);

        if constexpr (std::is_base_of_v<System::IUpdatable, T>)
        {
            mUpdatables.push_back(raw);
        }

        if constexpr (std::is_base_of_v<System::IDrawable, T>)
        {
            mDrawables.push_back(raw);
        }

        return raw;
    }

    template<typename T>
    T* getSystem()
    {
        static_assert(std::is_base_of_v<System::ISystem, T>, "T must derive from ISystem");
        auto it = mSystems.find(typeid(T));
        return (it != mSystems.end()) ? static_cast<T*>(it->second.get()) : nullptr;
    }

    Core::Renderer::VkRenderData& getRenderData()
    {
        return mRenderData;
    }

    void setPaused(bool paused) { mPaused = paused; }

  private:
    Engine() = default;
    ~Engine() = default;

    float mLastTickTime = 0.0;
    Timer mFrameTimer{};

    Core::Renderer::VkRenderData mRenderData;

    std::unordered_map<std::type_index, std::unique_ptr<System::ISystem>> mSystems;
    std::vector<System::IUpdatable*> mUpdatables;
    std::vector<System::IDrawable*> mDrawables;

    // TODO
    // replace it with EngineState enum
    bool mPaused = false;
};
}