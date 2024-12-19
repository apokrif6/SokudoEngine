#pragma once

#include "vector"
#include "EventListener.h"

class EventDispatcher
{
  public:
    void subscribe(EventListener* listener) { mListeners.push_back(listener); }

    void unsubscribe(EventListener* listener)
    {
        mListeners.erase(std::remove(mListeners.begin(), mListeners.end(), listener), mListeners.end());
    }

    void dispatch(const Event& event)
    {
        for (EventListener* listener : mListeners)
        {
            listener->onEvent(event);
        }
    }

  private:
    std::vector<EventListener*> mListeners;
};
