#pragma once

namespace Core
{
enum class EngineState
{
    Initializing, // startup engine. don't affect anything yet
    Running,      // normal state. update and draw are called
    Paused,       // update is not called, draw is called
    Loading       // update and draw are not called. e.g. used by scene system to process saving/loading scene
};
}