// vim:ts=3:sts=3:sw=3

#ifndef TEMPLATE_TUM_SESSION_H_
#define TEMPLATE_TUM_SESSION_H_

#include "xi/define.hxx"
#include "rp/member.hxx"
#include "stack/http1/request.hxx"
#include "stack/http1/response.hxx"
#include "template/ifm/timer_event.h"
#include "template/tum/define.h"
#include "template/main/define.h"

namespace tu {

class Fsm;

class Session : public xi::rp::Member
{
   friend class FsmIdle;
   friend class FsmApi1;
   friend class FsmApi2;

   public :
      Session();
      ~Session();

      virtual void         Clear();
      void                 Release();
      virtual xi::rp::result::e OnReceive(xi::rp::Payload &message);
      xi::rp::result::e    Send(xi::rp::Payload &message);

      const char          *Tag();
      uint32_t             NextSeq();

      void                 ChangeFsm(const char *ltag, tu::fsmtype::e type);
      const char          *GetFsmName();
                            
      bool                 SendHttpRequest(IN std::unique_ptr<xi::h1::Request> request, OUT xi::SocketAddr &assign_addr);
      bool                 SendHttpResponse(IN std::unique_ptr<xi::h1::Response> response);

      bool                 CheckTimerEvent(ifm::TimerEvent &timeout);
      bool                 TimerStart(td::timer::e timerType, uint32_t msec = 0);
      bool                 TimerStarted(td::timer::e timerType);
      bool                 TimerStop(td::timer::e timerType);
      bool                 TimerClear(td::timer::e timerType);
      void                 TimerAllStop();

      // 현재 라인 아래부터 멤버함수 추가한다.

   private :
      tu::fsmtype::e       fsm_type_;
      tu::Fsm             *fsm_object_;

      uint32_t             seqno_;
      xi::String           trace_tag_;
      test::scenario::e    test_scenario_;

      xi::timerid_t        timer_table_[td::timer::END_OF_ENUM];

      // 현재 라인 아래부터 멤버변수 추가한다.
      xi::msec_t           start_time_;
      uint32_t             tu_data_id_;
      unsigned             scenario_step_;
};

}

#endif
