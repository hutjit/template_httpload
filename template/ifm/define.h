// vim:ts=3:sts=3:sw=3

#ifndef TEMPLATE_IFM_DEFINE_H_
#define TEMPLATE_IFM_DEFINE_H_

namespace ifm { // interface-message


namespace msgtype {
   typedef enum {
      NONE = 0,
      SOCKET_EVENT,
      TIMER_EVENT,
      LOAD_COMMAND,
      HTTP1_PROTOCOL,
      KAFKA_PRODUCE,
      END_OF_ENUM
   } e;

   const char *name(e param);
   bool scope(e param);
   bool valid(e param);
}


}

#endif
