// vim:ts=3:sts=3:sw=3

#ifndef TEMPLATE_TUM_FSM_H_
#define TEMPLATE_TUM_FSM_H_

#include <string>
#include "stack/http1/request.hxx"
#include "stack/http1/response.hxx"
#include "template/ifm/load_command.h"
#include "template/ifm/timer_event.h"
#include "template/ifm/http1_protocol.h"
#include "template/tum/session.h"

namespace tu {

class Fsm {
   public :
      Fsm();
      virtual ~Fsm();

      xi::rp::result::e OnReceive(tu::Session &context, xi::rp::Payload &message);

   protected :
      virtual xi::rp::result::e ProcessTimeout(tu::Session &context, ifm::TimerEvent &timeout);
      virtual xi::rp::result::e ProcessLoadCommand(tu::Session &context, ifm::LoadCommand &loadcmd);

      virtual xi::rp::result::e ProcessHttpRequest(tu::Session &context, xi::h1::Request &request);
      virtual xi::rp::result::e ProcessHttpResponse(tu::Session &context, xi::h1::Response &response);
};

}

#endif
