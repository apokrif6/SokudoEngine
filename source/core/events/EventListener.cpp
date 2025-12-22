#include "EventListener.h"
#include "EventDispatcher.h"

void EventListener::subscribeToEventDispatcher(EventDispatcher& eventDispatcher) { eventDispatcher.subscribe(this); }
