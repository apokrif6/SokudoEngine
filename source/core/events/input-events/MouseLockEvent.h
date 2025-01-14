#pragma once

#include "core/events/Event.h"

struct MouseLockEvent final : public Event
{
    MouseLockEvent(const bool inIsLocked) : isLocked(inIsLocked) {}

    bool isLocked;
};