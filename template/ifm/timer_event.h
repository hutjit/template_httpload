#ifndef _IFM_TIMER_EVENT_H_
#define _IFM_TIMER_EVENT_H_

#include "rp/payload.hxx"
#include "template/ifm/define.h"

namespace ifm {


class TimerEvent : public xi::rp::Payload
{
   public :
      typedef enum {
         UNKNOWN,
         START,
         STOP,
         TIMEOUT
      } EventType;

   public :
      TimerEvent();
      virtual ~TimerEvent();

      virtual xi::rp::Payload  *Clone();

      EventType               GetEventType();
      xi::timerid_t           GetTimerId();
      uint32_t                GetExpires();
      int32_t                 GetParam1();

      void                    SetEventType(EventType type);
      void                    SetTimerId(xi::timerid_t id);
      void                    SetExpires(uint32_t msec);
      void                    SetParam1(int32_t param);
                           
   private :               
      EventType               event_type_;
      xi::timerid_t           timerid_;
      uint32_t                expires_;
      int32_t                 param1_;
};


}

#endif
