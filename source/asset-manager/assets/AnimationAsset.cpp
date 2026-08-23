#include "AnimationAsset.h"
#include "animations/AnimationsUtils.h"
#include "tools/Logger.h"

Core::Assets::AnimationAsset::AnimationAsset(const std::string& path)
    : Asset(path), mAnimation(Animations::AnimationsUtils::loadAnimationFromFile(path))
{
    if (mAnimation.channels.empty())
    {
        Logger::log(1, "Failed to load AnimationAsset: %s", path.c_str());
    }
}
