// vim:ts=3:sts=3:sw=3

#include "fsm_api_1.h"

#include "xi/logger.hxx"
#include "xi/util.hxx"
#include "template/hsm/manager.h"
#include "template/main/property.h"
#include "template/main/statistics.h"

namespace tu {


#define FN fn << context.Tag() << " "

FsmApi1::FsmApi1()
{
}

FsmApi1::~FsmApi1()
{
}

xi::rp::result::e FsmApi1::ProcessTimeout(tu::Session &context, ifm::TimerEvent &timeout)
{
   static const char *fn = "[tu::api1::ProcessTimeout] ";

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

      case td::timer::HTTP_RESPONSE_TIMEOUT :
         {
            WLOG(FN << td::timer::name(timertype));
            result = xi::rp::result::DO_RELEASE;
         } break;

      default :
         {
            WLOG(FN << "not-support timer:" << td::timer::name(timertype));
         } break;
   }

   return result;
}

xi::rp::result::e FsmApi1::ProcessHttpResponse(tu::Session &context, xi::h1::Response &response)
{
   const char *fn = "[tu::api1::ProcessHttpResponse] ";

   const char *echo_value = response.GetHeaderValue("Echo");
   int status_code = response.GetStatusCode();
   DLOG(FN << "step:" << api01step::name((api01step::e)context.scenario_step_) << " status-code:" << status_code << " Echo:" << echo_value);

   context.TimerStop(td::timer::HTTP_RESPONSE_TIMEOUT);

   if (context.trace_tag_.IsEqual(echo_value)) {
      ih::Statistics::Instance()->StatService(true, ih::stat::service::API01, status_code);
   } else {
      ih::Statistics::Instance()->StatService(true, ih::stat::service::API01, status_code + 9000);
   }

   // unsigned curstep = context.scenario_step_;
   // context.scenario_step_ = api01step::next((api01step::e)curstep);
   // DLOG(FN << "step:" << api01step::name((api01step::e)curstep) << " next:" << api01step::name((api01step::e)context.scenario_step_));
   // return DoNextStep(context);


   return xi::rp::result::DO_RELEASE;
}

xi::rp::result::e FsmApi1::DoNextStep(tu::Session &context)
{
   const char *fn = "[tu::api1::DoNextStep] ";

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

xi::rp::result::e FsmApi1::SendRequest(tu::Session &context)
{
   const char *fn = "[tu::api1::SendRequest] ";

   xi::String scheme;
   xi::String req;
   xi::String content;

   if (false == hs::Manager::Instance()->Next(test::scenario::name(context.test_scenario_), scheme, req, content)) {
      return xi::rp::result::DO_RELEASE;
   }


   char payload_len[32];
   snprintf(payload_len, sizeof(payload_len), "%u", content.GetSize());

   std::unique_ptr<xi::h1::Request> request(new xi::h1::Request(req.c_str(), req.GetSize(), scheme.c_str()));
   request->AddHeader("Content-Length", payload_len);

   if (false == content.IsEmpty())
      request->SetPayload((const uint8_t*)content.c_str(), content.GetSize());

   ifm::Http1Protocol proto;
   proto.PushRequest(std::move(request));

   if (false == context.TimerStart(td::timer::HTTP_RESPONSE_TIMEOUT)) {
      WLOG(FN << "TimerStart(HTTP_RESPONSE_TIMEOUT) fail");
      return xi::rp::result::DO_RELEASE;
   }

   if (xi::rp::result::SUCCESS != context.Send(proto)) {
      WLOG(FN << "Send() fail");
      return xi::rp::result::DO_RELEASE;
   } 

   ih::Statistics::Instance()->StatService(false, ih::stat::service::API01, 0);

   return xi::rp::result::SUCCESS;
}


}
