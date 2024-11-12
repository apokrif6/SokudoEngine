#pragma once

#include "events/Event.h"

struct MouseMovementEvent : public Event
{
    MouseMovementEvent(int inDeltaX, int inDeltaY) : deltaX(inDeltaX), deltaY(inDeltaY) {}

    int deltaX;
    int deltaY;
};
