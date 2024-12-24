#pragma once

#include "Event.h"

class EventListener
{
  public:
    virtual ~EventListener() = default;

    virtual void onEvent(const Event& event) = 0;
};
