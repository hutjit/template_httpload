// vim:ts=3:sts=3:sw=3

#ifndef TEMPLATE_TUM_FSM_API_1_H_
#define TEMPLATE_TUM_FSM_API_1_H_

#include "xi/singleton.hxx"
#include "template/tum/fsm.h"

namespace tu {


class FsmApi1 : public Fsm, public xi::Singleton<FsmApi1>
{
   public :
      FsmApi1();
      virtual ~FsmApi1();

   protected :
      virtual xi::rp::result::e ProcessTimeout(tu::Session &context, ifm::TimerEvent &timeout);
      virtual xi::rp::result::e ProcessHttpResponse(tu::Session &context, xi::h1::Response &response);

   private :
      xi::rp::result::e DoNextStep(tu::Session &context);
      xi::rp::result::e SendRequest(tu::Session &context);
};


}

#endif
