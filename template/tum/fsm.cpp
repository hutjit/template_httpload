// vim:ts=3:sts=3:sw=3

#include "fsm.h"
#include <typeinfo>
#include "xi/logger.hxx"
#include "session.h"
#include "template/main/statistics.h"

namespace tu {


#define FN fn << context.Tag()

Fsm::Fsm()
{
}

Fsm::~Fsm()
{
}

xi::rp::result::e Fsm::OnReceive(tu::Session &context, xi::rp::Payload &message)
{
   static const char *fn = "[tu::FSM::OnReceive] ";

   switch (message.GetType()) {
      case ifm::msgtype::TIMER_EVENT :
         {
            ifm::TimerEvent &timeout = static_cast<ifm::TimerEvent&>(message);
            if (ifm::TimerEvent::TIMEOUT != timeout.GetEventType()) {
               WLOG(FN << " invalid EventType:" << timeout.GetEventType());
               return xi::rp::result::UNSUPPORTED_FEATURE;
            }

            td::timer::e timertype = (td::timer::e) timeout.GetParam1();
            if (false == context.TimerClear(timertype)) {
               WLOG(FN << " race condition. already stopped timer:" << td::timer::name(timertype));
               return xi::rp::result::SUCCESS;
            }

            return ProcessTimeout(context, timeout);
         } break;

      case ifm::msgtype::LOAD_COMMAND :
         {
            ifm::LoadCommand &loadcmd = static_cast<ifm::LoadCommand&>(message);
            return ProcessLoadCommand(context, loadcmd);
         } break;

      case ifm::msgtype::HTTP1_PROTOCOL :
         {
            ifm::Http1Protocol &proto = static_cast<ifm::Http1Protocol&>(message);
            if (proto.GetRequest())
               return ProcessHttpRequest(context, *proto.GetRequest());
            else if (proto.GetResponse())
               return ProcessHttpResponse(context, *proto.GetResponse());
            else
               return xi::rp::result::INVALID_ARGS;
         } break;

      default :
         {
            WLOG(FN << " not support type:" << typeid(message).name());
            return xi::rp::result::UNSUPPORTED_FEATURE;
         } break;
   }

   return xi::rp::result::SUCCESS;
}

xi::rp::result::e Fsm::ProcessTimeout(tu::Session &context, ifm::TimerEvent &timeout)
{
   static const char *fn = "[tu::FSM::ProcessTimeout] ";
   WLOG(FN << " invalid-state[" << context.GetFsmName() << "] timerid:" << timeout.GetTimerId());
   ih::Statistics::Instance()->StatDebug(ih::stat::debug::INVALID_FSM);
   return xi::rp::result::INVALID_STATE;
}

xi::rp::result::e Fsm::ProcessLoadCommand(tu::Session &context, ifm::LoadCommand &loadcmd)
{
   static const char *fn = "[tu::FSM::ProcessLoadCommand] ";
   WLOG(FN << " invalid-state[" << context.GetFsmName() << "] scenario:" << test::scenario::name((test::scenario::e)loadcmd.GetScenario()));
   return xi::rp::result::INVALID_STATE;
}

xi::rp::result::e Fsm::ProcessHttpRequest(tu::Session &context, xi::h1::Request &request)
{
   static const char *fn = "[tu::FSM::ProcessHttpRequest] ";
   WLOG(FN << " invalid-state[" << context.GetFsmName() << "] " << xi::h1::method::name(request.GetMethod()));
   ih::Statistics::Instance()->StatDebug(ih::stat::debug::INVALID_FSM);
   return xi::rp::result::INVALID_STATE;
}

xi::rp::result::e Fsm::ProcessHttpResponse(tu::Session &context, xi::h1::Response &response)
{
   static const char *fn = "[tu::FSM::ProcessHttpResponse] ";
   WLOG(FN << " invalid-state[" << context.GetFsmName() << "] " << response.GetStatusCode());
   ih::Statistics::Instance()->StatDebug(ih::stat::debug::INVALID_FSM);
   return xi::rp::result::INVALID_STATE;
}


}
