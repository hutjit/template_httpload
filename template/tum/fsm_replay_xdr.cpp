#include "fsm_replay_xdr.h"

#include "xi/logger.hxx"
#include "xi/splitter.hxx"
#include "xi/util.hxx"
#include "template/ifm/kafka_produce.h"
#include "template/hsm/replay_xdrs.h"
#include "template/main/property.h"
#include "template/main/statistics.h"

namespace tu {


#define FN fn << context.Tag() << " "

FsmReplayXdr::FsmReplayXdr()
{
}

FsmReplayXdr::~FsmReplayXdr()
{
}

xi::rp::result::e FsmReplayXdr::ProcessTimeout(tu::Session &context, ifm::TimerEvent &timeout)
{
   static const char *fn = "[tu::repaly::ProcessTimeout] ";

   td::timer::e timertype = (td::timer::e) timeout.GetParam1();
   xi::rp::result::e result = xi::rp::result::SUCCESS;

   DLOG(FN << td::timer::name(timertype));

   switch (timertype)
   {
      case td::timer::SESSION_INACTIVITY_TIMEOUT :
         { 
            WLOG(FN << td::timer::name(timertype));
            result = xi::rp::result::DO_RELEASE;
         } break;

      case td::timer::TRIGGER_SCENARIO :
         { 
            context.scenario_step_ = api01step::next((api01step::e)context.scenario_step_);
            DLOG(FN << "step:" << api01step::name((api01step::e)context.scenario_step_));
            result = DoNextStep(context);
         } break;

      default :
         {
            WLOG(FN << "not-support timer:" << td::timer::name(timertype));
         } break;
   }

   return result;
}

xi::rp::result::e FsmReplayXdr::DoNextStep(tu::Session &context)
{
   const char *fn = "[tu::repaly::DoNextStep] ";

   DLOG(FN << "step:" << api01step::name((api01step::e)context.scenario_step_));

   xi::rp::result::e rv = xi::rp::result::SUCCESS;
   switch (context.scenario_step_) {
      case api01step::S1_SEND_REQUEST :
         rv = SendRequest(context);
         break;

      case api01step::COMPLETE :
         rv = xi::rp::result::DO_RELEASE;
         break;

      default :
         WLOG(FN << "unsupport step:" << api01step::name((api01step::e)context.scenario_step_));
         rv = xi::rp::result::DO_RELEASE;
         break;
   }

   return rv;
}

xi::rp::result::e FsmReplayXdr::SendRequest(tu::Session &context)
{
   const char *fn = "[tu::repaly::SendRequest] ";

   WLOG(FN << "TODO");

   return xi::rp::result::DO_RELEASE;
}


}
