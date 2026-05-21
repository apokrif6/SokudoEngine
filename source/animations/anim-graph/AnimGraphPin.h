#pragma once

#include "uuid.h"

namespace Core::Animations
{
using PinID = uint64_t;

enum class AnimGraphPinKind
{
    Input,
    Output
};

enum class AnimGraphValueType
{
    Pose,
    Float,
    Bool
};

struct AnimGraphPin
{
    PinID id{};
    AnimGraphPinKind kind{};
    AnimGraphValueType type{};
};
} // namespace Core::Animations