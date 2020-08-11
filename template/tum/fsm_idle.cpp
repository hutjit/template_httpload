#include "fsm_idle.h"

#include "rapidjson/document.h"
#include "rapidjson/writer.h"
#include "rapidjson/stringbuffer.h"
#include "xi/logger.hxx"
#include "xi/splitter.hxx"
#include "xi/util.hxx"
#include "template/main/property.h"

namespace tu {


#define FN fn << context.Tag() << " "

FsmIdle::FsmIdle()
{
}

FsmIdle::~FsmIdle()
{
}

xi::rp::result::e FsmIdle::ProcessLoadCommand(tu::Session &context, ifm::LoadCommand &loadcmd)
{
   static const char *fn = "[tu::idle::ProcessLoadCommand] ";

   context.test_scenario_ = (test::scenario::e)loadcmd.GetScenario();

   DLOG(FN << "scenario:" << test::scenario::name(context.test_scenario_) << " user:" << loadcmd.GetUser());

   if (false == context.TimerStart(td::timer::SESSION_INACTIVITY_TIMEOUT)) {
      WLOG(FN << "TimerStart(SESSION_INACTIVITY_TIMEOUT) fail");
      return xi::rp::result::DO_RELEASE;
   }

   tu::fsmtype::e fsm = tu::fsmtype::IDLE;

   switch (loadcmd.GetScenario())
   {
      case test::scenario::API1 :
         fsm = tu::fsmtype::API_1;
         break;

      case test::scenario::REPLAY_XDR :
         fsm = tu::fsmtype::REPLAY_XDR;
         break;

      default :
         WLOG(FN << "unsupported scenario:" << test::scenario::name((test::scenario::e)loadcmd.GetScenario()));
         return xi::rp::result::DO_RELEASE;
         break;
   }

   if (false == context.TimerStart(td::timer::TRIGGER_SCENARIO)) {
      WLOG(FN << "TimerStart(TRIGGER_SCENARIO) fail");
      return xi::rp::result::DO_RELEASE;
   }

   //context.tu_data_id_ = ih::LoadScript::Instance()->NextSeq(context.test_scenario_);
   context.trace_tag_.Csnprintf(32, "id:%u[s:%u]", context.tu_data_id_, context.GetSessionId());
   context.ChangeFsm(fn, fsm);

   return xi::rp::result::SUCCESS;
}
xi::rp::result::e FsmIdle::ProcessHttpRequest(tu::Session &context, xi::h1::Request &request)
{
   static const char *fn = "[tu::idle::ProcessHttpRequest] ";

   DLOG(FN << "path:" << request.GetPath());
   SendFinalResponse(context, request, 200);

   return xi::rp::result::DO_RELEASE;
}

bool FsmIdle::SendFinalResponse(IN tu::Session &context, IN xi::h1::Request &request, IN uint16_t status_code)
{
   static const char *fn = "[tu::idle::SendFinalResponse] ";

   const char *echo_value = request.GetHeaderValue("Echo");
   const char *connection_header_value = request.GetHeaderValue("Connection");

   std::unique_ptr<xi::h1::Response> response(new xi::h1::Response(request, status_code));
   if (echo_value)
      response->AddHeader("Echo", echo_value);
   if (connection_header_value)
      response->AddHeader("Connection", connection_header_value);
   response->AddHeader("Content-Length", "0");
   if (429 == status_code) 
      response->AddHeader("Retry-After", "5");

   return context.SendHttpResponse(std::move(response));
}


}

