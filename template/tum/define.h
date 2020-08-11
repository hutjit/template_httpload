#ifndef _TUM_DEFINE_H_
#define _TUM_DEFINE_H_


namespace tu {

namespace fsmtype {
   typedef enum {
      NONE = 0,
      IDLE,
      API_1,
      REPLAY_XDR,
      END_OF_ENUM
   } e;

   const char *name(e type);
}

namespace api01step {
   typedef enum {
      NONE = 0,
      S1_SEND_REQUEST,
      S2_WAIT_REPLY,
      COMPLETE,
      END_OF_ENUM
   } e;
   const char *name(e value);
   bool        scope(e value);
   e           next(e value);
}


}

#endif
