// vim:ts=3:sts=3:sw=3

#include "timer_event.h"

namespace ifm {

TimerEvent::TimerEvent() : xi::rp::Payload(ifm::msgtype::TIMER_EVENT)
{
   event_type_ = UNKNOWN;
   timerid_ = 0;
   expires_ = 0;
   param1_ = 0;
}

TimerEvent::~TimerEvent()
{
}

xi::rp::Payload *TimerEvent::Clone()
{
   TimerEvent *pClone = new TimerEvent();
   *pClone = *this;

   return pClone;
}

TimerEvent::EventType TimerEvent::GetEventType()
{
   return event_type_;
}

xi::timerid_t TimerEvent::GetTimerId()
{
   return timerid_;
}

uint32_t TimerEvent::GetExpires()
{
   return expires_;
}

int32_t TimerEvent::GetParam1()
{
   return param1_;
}

void TimerEvent::SetEventType(EventType type)
{
   event_type_ = type;
}

void TimerEvent::SetTimerId(xi::timerid_t id)
{
   timerid_ = id;
}

void TimerEvent::SetExpires(uint32_t msec)
{
   expires_ = msec;
}

void TimerEvent::SetParam1(int32_t param)
{
   param1_ = param;
}

}
