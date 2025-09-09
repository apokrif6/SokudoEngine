#pragma once

#include "core/events/Event.h"

struct KeyEvent final : Event
{
    KeyEvent(const int inKey, const int inScancode, const int inAction, const int inMods)
        : key(inKey), scancode(inScancode), action(inAction), mods(inMods)
    {
    }

    int key;
    int scancode;
    int action;
    int mods;
};
