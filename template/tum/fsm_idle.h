#ifndef _TUM_FSM_IDLE_H_
#define _TUM_FSM_IDLE_H_

#include "xi/singleton.hxx"
#include "template/tum/fsm.h"

namespace tu {


class FsmIdle : public Fsm, public xi::Singleton<FsmIdle>
{
   public :
      FsmIdle();
      virtual ~FsmIdle();

   protected :
      virtual xi::rp::result::e ProcessLoadCommand(tu::Session &context, ifm::LoadCommand &loadcmd);
      virtual xi::rp::result::e ProcessHttpRequest(tu::Session &context, xi::h1::Request &request);

   private :
      bool SendFinalResponse(IN tu::Session &context, IN xi::h1::Request &request, IN uint16_t status_code);
};


}

#endif
