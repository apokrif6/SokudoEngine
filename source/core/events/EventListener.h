#pragma once

#include "Event.h"

class EventDispatcher;

class EventListener
{
public:
    virtual ~EventListener() = default;

    void subscribeToEventDispatcher(EventDispatcher& eventDispatcher);

    virtual void onEvent(const Event& event) = 0;
};
