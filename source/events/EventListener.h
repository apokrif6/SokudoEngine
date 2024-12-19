#pragma once

#include "Event.h"

class EventListener
{
  public:
    virtual void onEvent(const Event& event) = 0;
};