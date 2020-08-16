// vim:ts=3:sts=3:sw=3

#ifndef TEMPLATE_MAIN_DEFINE_H_
#define TEMPLATE_MAIN_DEFINE_H_

#include <stdint.h>

#define _PROCESS_VERSION "nrftester-v0.1.0"

namespace td {

   namespace functype {
      typedef enum {
         NONE = 0,
         IH_SESSION,
         TU_SESSION,
         END_OF_ENUM
      } e;

      const char *name(functype::e type);
   }

   namespace timer {
      typedef enum {
         NONE = 0,
         TRIGGER_SCENARIO,
         TRIGGER_UNSUBSCRIPTION,
         SESSION_INACTIVITY_TIMEOUT,
         HTTP_RESPONSE_TIMEOUT,
         NOTIFY_REQUEST_TIMEOUT,
         END_OF_ENUM
      } e;

      const char *name(e type);
      bool        scope(e type);
      e           code(const char *name);
      bool        valid(e type);
      uint32_t    default_value(e type);
   }

   namespace loadcmd {
      typedef enum {
         NONE = 0,
         START,
         STOP,
         UNIT_TEST,
         END_OF_ENUM
      } e;
      const char *name(loadcmd::e type);
   }

}

namespace test {

   namespace scenario {
      typedef enum {
         NONE = 0,
         API1,
         END_OF_ENUM
      } e;

      const char *name(scenario::e type);
      bool        scope(e type);
      e           code(const char *name);
   }

   class Load {
      public :
         scenario::e scenario_;
         uint32_t    turn_;

         Load();
   };

   namespace flag {

      namespace toggle {
         typedef enum {
            NONE = 0,
            TPT_PLAN,
            TR_PER_MINUTE,
            END_OF_ENUM
         } e;

         const char *name(e type);
         bool        scope(e type);
         e           code(const char *name);
         bool        default_value(e type);
      }

      namespace number {
         typedef enum {
            NONE = 0,
            KEEP_CONNECTIONS,
            END_OF_ENUM
         } e;

         const char *name(e type);
         bool        scope(e type);
         e           code(const char *name);
         int         default_value(e type);
      }

      namespace letter {
         typedef enum {
            NONE = 0,
            TPT_PLAN_FILE,
            END_OF_ENUM
         } e;

         const char *name(e type);
         bool        scope(e type);
         e           code(const char *name);
         const char *default_value(e type);
      }

   }

}

#endif
