#ifndef _TUM_FSM_REPLAY_XDR_H_
#define _TUM_FSM_REPLAY_XDR_H_

#include "xi/singleton.hxx"
#include "template/tum/fsm.h"

namespace tu {


class FsmReplayXdr : public Fsm, public xi::Singleton<FsmReplayXdr>
{
   public :
      FsmReplayXdr();
      virtual ~FsmReplayXdr();

   protected :
      virtual xi::rp::result::e ProcessTimeout(tu::Session &context, ifm::TimerEvent &timeout);

   private :
      xi::rp::result::e DoNextStep(tu::Session &context);
      xi::rp::result::e SendRequest(tu::Session &context);
};


}

#endif
