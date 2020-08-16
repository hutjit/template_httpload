// vim:ts=3:sts=3:sw=3

#include "define.h"
#include "xi/util.hxx"

namespace ifm {


namespace msgtype {
   static const char *nametag[] =
   {
      "(none)",
      "SOCKET_EVENT",
      "TIMER_EVENT",
      "LOAD_COMMAND",
      "HTTP1_PROTOCOL",
      "KAFKA_PRODUCE",
   };

   const char *name(e param)
   {
      if (scope(param))
         return nametag[param];

      return "(critical-exception)";
   }

   bool scope(e param)
   {
      if (END_OF_ENUM > (unsigned)param)
         return true;
      return false;
   }

   bool valid(e param)
   {
      if (param && (END_OF_ENUM > (unsigned)param))
         return true;
      return false;
   }

}


}
