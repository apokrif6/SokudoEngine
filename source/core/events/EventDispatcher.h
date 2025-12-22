#pragma once

#include <vector>
#include "EventListener.h"

class EventDispatcher
{
public:
    void subscribe(EventListener* listener) { mListeners.push_back(listener); }

    void unsubscribe(EventListener* listener) { std::erase(mListeners, listener); }

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
