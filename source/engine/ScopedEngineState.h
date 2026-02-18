#pragma once

#include "Engine.h"

namespace Core
{
class ScopedEngineState
{
public:
    ScopedEngineState(EngineState state)
    {
        Engine& engine = Engine::getInstance();
        mPreviousState = engine.getState();
        engine.setState(state);
    }

    ~ScopedEngineState()
    {
        Engine& engine = Engine::getInstance();
        engine.setState(mPreviousState);
    }

private:
    EngineState mPreviousState;
};
} // namespace Core