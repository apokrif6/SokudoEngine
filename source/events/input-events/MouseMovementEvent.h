#pragma once

#include "events/Event.h"

struct MouseMovementEvent final : Event
{
    explicit MouseMovementEvent(const int inDeltaX, const int inDeltaY) : deltaX(inDeltaX), deltaY(inDeltaY) {}

    int deltaX;
    int deltaY;
};
