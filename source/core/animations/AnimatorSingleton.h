#pragma once

#include "assimp/scene.h"
#include "assimp/Importer.hpp"
#include <map>
#include <memory>

// TODO
// remove singleton from name when logic from animation will land here!

// TODO
// create templated class for all singletons
namespace Core::Animations
{
class AnimatorSingleton
{
  public:
    static AnimatorSingleton& getInstance()
    {
        static AnimatorSingleton instance;
        return instance;
    }

    AnimatorSingleton(const AnimatorSingleton&) = delete;
    AnimatorSingleton& operator=(const AnimatorSingleton&) = delete;

    // TODO
    // dude refactor this shit ( ｡ •̀ ᴖ •́ ｡)
    std::map<std::string, std::shared_ptr<Assimp::Importer>> importers;

    AnimatorSingleton() = default;
    ~AnimatorSingleton() = default;
};
} // namespace Core::Animations
