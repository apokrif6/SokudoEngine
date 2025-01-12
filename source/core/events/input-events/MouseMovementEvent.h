#pragma once

#include "core/events/Event.h"

struct MouseMovementEvent final : public Event
{
    MouseMovementEvent(const int inDeltaX, const int inDeltaY) : deltaX(inDeltaX), deltaY(inDeltaY) {}

    int deltaX;
    int deltaY;
};
