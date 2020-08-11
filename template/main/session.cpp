// vim:ts=3:sts=3:sw=3

#include "session.h"
#include <memory>
#include <typeinfo>
#include "xi/logger.hxx"
#include "rp/member.hxx"
#include "template/ifm/timer_event.h"
#include "template/tum/manager.h"
#include "template/main/interface_hub.h"

namespace ih {


Session::Session() : xi::rp::Session(td::functype::IH_SESSION)
{
}

Session::~Session()
{
}

xi::rp::result::e Session::OnReceive(xi::rp::Payload &message)
{
   static const char *FN = "[ih::Session::OnReceive] ";
   // Session release는 여기가 마지노선이다. interface_hub에서는 release 처리하지 않는다.

   message.SetSessReference(this);


   // ----- Session Lock -----
   xi::Mutex::ScopedLock lock(lock_);


   xi::rp::membid_t mid = message.GetDstMembId();

   if (mid) {
      xi::rp::Member *member = xi::rp::Session::Find(mid);
      if (NULL == member) {
         WLOG(FN << "not-found sid:" << message.GetDstSessId() << " mid:" << mid << " typeid:" << typeid(message).name());
         return xi::rp::result::RESOURCE_NOT_FOUND;
      }

      xi::rp::result::e result = member->OnReceive(message);
      if (xi::rp::result::DO_RELEASE == result) {
         DLOG(FN << "result:" << xi::rp::result::name(result));
         Release();
         return xi::rp::result::SUCCESS;
      }

      return result;
   }

   switch (message.GetType()) {
      case ifm::msgtype::LOAD_COMMAND :
      case ifm::msgtype::HTTP1_PROTOCOL :
         {
            xi::rp::membid_t mid;
            xi::rp::result::e result = tu::Manager::Instance()->OnReceive(message, mid);
            if (xi::rp::result::SUCCESS != result) {
               DLOG(FN << "msg:" << ifm::msgtype::name((ifm::msgtype::e)message.GetType()) << " tum-result:" << xi::rp::result::name(result));
               Release();
            }
         } break;
		 
      default :
         {
            WLOG(FN << "MsgType:" << typeid(message).name() << " does not support.");
         } break;
   }

   return xi::rp::result::SUCCESS;
}

xi::rp::result::e Session::Send(xi::rp::Payload &message)
{
   static InterfaceHub *msghub = InterfaceHub::Instance();
   return msghub->Send(message);
}

xi::timerid_t Session::StartTimer(uint32_t msec, int32_t tevent, xi::rp::membid_t mid)
{
   static InterfaceHub *msghub = InterfaceHub::Instance();

   ifm::TimerEvent timer_event;
   timer_event.SetSrcSessId(GetSessionId());
   timer_event.SetSrcMembId(mid);

   timer_event.SetEventType(ifm::TimerEvent::START);
   timer_event.SetExpires(msec);
   timer_event.SetParam1(tevent);

   return msghub->StartTimer(timer_event);
}

bool Session::StopTimer(xi::timerid_t timerid)
{
   static InterfaceHub *msghub = InterfaceHub::Instance();

   ifm::TimerEvent timer_event;
   timer_event.SetEventType(ifm::TimerEvent::STOP);
   timer_event.SetTimerId(timerid);

   return msghub->StopTimer(timer_event);
}


}
