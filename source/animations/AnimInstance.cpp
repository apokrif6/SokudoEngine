#include "AnimInstance.h"

#include "AnimParamID.h"
#include "AnimationsData.h"
#include "anim-graph/AnimationContext.h"

Core::Animations::Pose Core::Animations::AnimInstance::evaluate(AnimationContext& context)
{
    context.instance = this;

    return mGraph->evaluate(context);
}

void Core::Animations::AnimInstance::setFloat(const std::string& name, float value)
{
    const uint32_t id = AnimParamID(name.c_str()).id;
    mFloatParameters[id] = value;
}

void Core::Animations::AnimInstance::setBool(const std::string& name, bool value)
{
    const uint32_t id = AnimParamID(name.c_str()).id;
    mBoolParameters[id] = value;
}

float Core::Animations::AnimInstance::getFloat(const std::string& name) const
{
    const uint32_t id = AnimParamID(name.c_str()).id;

    if (const auto it = mFloatParameters.find(id); it != mFloatParameters.end())
    {
        return it->second;
    }

    return 0.0f;
}

bool Core::Animations::AnimInstance::getBool(const std::string& name) const
{
    const uint32_t id = AnimParamID(name.c_str()).id;

    if (const auto it = mBoolParameters.find(id); it != mBoolParameters.end())
    {
        return it->second;
    }

    return false;
}

void Core::Animations::AnimInstance::setFloat(const AnimParamID id, const float value)
{
    mFloatParameters[id.id] = value;
}

void Core::Animations::AnimInstance::setBool(const AnimParamID id, const bool value) { mBoolParameters[id.id] = value; }

float Core::Animations::AnimInstance::getFloat(const AnimParamID id) const
{
    if (const auto it = mFloatParameters.find(id.id); it != mFloatParameters.end())
    {
        return it->second;
    }

    return 0.0f;
}

bool Core::Animations::AnimInstance::getBool(AnimParamID id) const
{
    if (const auto it = mBoolParameters.find(id.id); it != mBoolParameters.end())
    {
        return it->second;
    }

    return false;
}