#pragma once

#include "AnimGraphPin.h"

namespace Core::Animations
{
using LinkID = uint64_t;

struct AnimGraphLink
{
    LinkID id{};
    PinID startPin{};
    PinID endPin{};
};
} // namespace Core::Animations