// vim:ts=3:sts=3:sw=3

#ifndef TEMPLATE_MAIN_SESSION_H_
#define TEMPLATE_MAIN_SESSION_H_ 

#include "rp/session.hxx"

namespace ih {


class Session : public xi::rp::Session
{
   public :
      Session();
      virtual ~Session();

      virtual xi::rp::result::e OnReceive(xi::rp::Payload &message);
      virtual xi::rp::result::e Send(xi::rp::Payload &message);

      xi::timerid_t StartTimer(uint32_t msec, int32_t tevent, xi::rp::membid_t mid);
      bool StopTimer(xi::timerid_t timerid);
};


}
#endif
