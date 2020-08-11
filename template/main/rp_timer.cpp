// vim:ts=3:sts=3:sw=3

#include "rp_timer.h"
#include "xi/logger.hxx"
#include "template/ifm/timer_event.h"
#include "interface_hub.h"

namespace ih {


RpTimer::RpTimer(InterfaceHub *callback) : xi::rp::Timer(1800, 200000), callback_(callback)
{
}

RpTimer::~RpTimer()
{
}

xi::timerid_t RpTimer::StartTimer(xi::rp::Payload &message)
{
   //static const char *FN = "[TMR::StartTimer] ";

   xi::rp::sessid_t sess_id = message.GetSrcSessId();
   xi::rp::membid_t memb_id = message.GetSrcMembId();

   if (ifm::msgtype::TIMER_EVENT != message.GetType())
      abort();

   ifm::TimerEvent &timer_event = static_cast<ifm::TimerEvent&>(message);

   if (ifm::TimerEvent::START != timer_event.GetEventType())
      abort();

   return xi::rp::Timer::StartTimer(timer_event.GetExpires(), sess_id, memb_id, timer_event.GetParam1());
}

bool RpTimer::StopTimer(xi::rp::Payload &message)
{
   //static const char *FN = "[TMR::StopTimer] ";

   if (ifm::msgtype::TIMER_EVENT != message.GetType())
      abort();

   ifm::TimerEvent &timer_event = static_cast<ifm::TimerEvent&>(message);

   if (ifm::TimerEvent::STOP != timer_event.GetEventType())
      abort();

   return xi::rp::Timer::StopTimer(timer_event.GetTimerId());
}

void RpTimer::OnTimeout(xi::timerid_t timerid, xi::rp::sessid_t sessid, xi::rp::membid_t membid, uint32_t tuparam)
{
   //static const char *FN = "[TMR::OnTimeout] ";

   //DLOG(FN << "DEBUG timerid:" << timerid << " sid:" << sessid << " mid:" << membid << " param:" << tuparam);

   ifm::TimerEvent *timer_event = new ifm::TimerEvent;
   timer_event->SetDstSessId(sessid);
   timer_event->SetDstMembId(membid);
   timer_event->SetEventType(ifm::TimerEvent::TIMEOUT);
   timer_event->SetTimerId(timerid);
   timer_event->SetParam1(tuparam);

   std::unique_ptr<xi::rp::Payload> primitive(timer_event);
   callback_->ExecThread(std::move(primitive));
}


}
