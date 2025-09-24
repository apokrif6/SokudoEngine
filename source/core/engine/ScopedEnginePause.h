#pragma once

#include "Engine.h"

namespace Core
{
class ScopedEnginePause
{
public:
    ScopedEnginePause()
    {
        Engine& engine = Engine::getInstance();
        engine.setPaused(true);
    }

    ~ScopedEnginePause()
    {
        Engine& engine = Engine::getInstance();
        engine.setPaused(false);
    }
};
} // namespace Core