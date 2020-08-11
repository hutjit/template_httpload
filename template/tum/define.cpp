#include "define.h"

namespace tu
{


namespace fsmtype
{
   const char *name(e type)
   {
      static const char *nametag[] =
      {
         "(none)",
         "IDLE",
         "API_1",
         "REPLAY_XDR",
      };

      if (END_OF_ENUM > (unsigned)type)
         return nametag[type];

      return "(critical-exception)";
   }
}

namespace api01step {
   static const char *nametag[] =
   {
      "(none)",
      "S1_SEND_REQUEST",
      "S2_WAIT_REPLY",
      "COMPLETE",
   };

   const char *name(e type)
   {
      if (END_OF_ENUM > (unsigned)type)
         return nametag[type];

      return "(critical-exception)";
   }

   bool scope(e type)
   {
      if (END_OF_ENUM > (unsigned)type)
         return true;
      return false;
   }

   e next(e value)
   {
      e rv = (e)(1 + value);
      return (scope(rv) ? rv : value);
   }
}


}
