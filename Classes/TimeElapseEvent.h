#ifndef _TIME_ELAPSE_EVENT_H_
#define _TIME_ELAPSE_EVENT_H_
#include "Event.h"
#include "GlobalState.h"
class TimeElapseEvent: public Event
{
public:
    void happen();
};
#endif
