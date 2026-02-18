#pragma once

#include "events/Event.h"

struct MouseLockEvent final : Event
{
    explicit MouseLockEvent(const bool inIsLocked) : isLocked(inIsLocked) {}

    bool isLocked;
};