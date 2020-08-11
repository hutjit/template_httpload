// vim:ts=3:sts=3:sw=3

#ifndef _MAIN_RP_TIMER_H_
#define _MAIN_RP_TIMER_H_ 

#include "rp/timer.hxx"
#include "rp/payload.hxx"

namespace ih {


class InterfaceHub;

class RpTimer : public xi::rp::Timer {
   public :
      RpTimer(InterfaceHub *callback);
      virtual ~RpTimer();

      xi::timerid_t StartTimer(xi::rp::Payload &message);
      bool StopTimer(xi::rp::Payload &message);
      virtual void OnTimeout(xi::timerid_t timerid, xi::rp::sessid_t sessid, xi::rp::membid_t membid, uint32_t tuparam);

   private :
      InterfaceHub *callback_;
};


}
#endif
