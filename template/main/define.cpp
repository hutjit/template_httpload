// vim:ts=3:sts=3:sw=3

#include "define.h"
#include "xi/logger.hxx"
#include "xi/util.hxx"


namespace td {

   namespace functype {
      const char *name(functype::e type)
      {
         static const char *nametag[] =
         {
            "(none)",
            "IH_SESSION",
            "TU_SESSION",
         };

         if (END_OF_ENUM > (unsigned)type)
            return nametag[type];

         return "(critical-exception)";
      }
   }

   namespace timer {
      static const char *nametag[] =
      {
         "(none)",
         "trigger.scenario",
         "trigger.unsubscription",
         "session.inactivity.timeout",
         "http.response.timeout",
         "notify.request.timeout",
      };

      const char *name(e type)
      {
         if (scope(type))
            return nametag[type];

         return "(critical-exception)";
      }

      bool scope(e type)
      {
         if (END_OF_ENUM > (unsigned)type)
            return true;
         return false;
      }

      e code(const char *name)
      {
         for (unsigned idx = (unsigned)(NONE + 1); idx < (unsigned) END_OF_ENUM; ++idx) {
            if (xi::StrEqualNoCase(nametag[idx], name)) {
               return (e) idx;
            }
         }

         return NONE;
      }

      bool valid(e type)
      {
         if (type && (END_OF_ENUM > (unsigned)type))
            return true;
         return false;
      }

      uint32_t default_value(e type)
      {
         switch (type) {
            case TRIGGER_UNSUBSCRIPTION      : { return 5000;     } break;
            case SESSION_INACTIVITY_TIMEOUT  : { return 600000;   } break;
            default                          : { return 5000;     } break;
         }

         return 5000;
      }
   }

   namespace loadcmd {
      const char *name(loadcmd::e type)
      {
         static const char *nametag[] =
         {
            "(none)",
            "START",
            "STOP",
            "UNIT_TEST",
         };

         if (END_OF_ENUM > (unsigned)type) {
            return nametag[type];
         }

         return "(critical-exception)";
      }
   }

}

namespace test {

   namespace scenario {
      static const char *nametag[] =
      {
         "(none)",
         "API1",
         "REPLAY_XDR",
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

      e code(const char *name)
      {
         for (unsigned idx = (unsigned)(NONE + 1); idx < (unsigned) END_OF_ENUM; ++idx) {
            if (xi::StrEqualNoCase(nametag[idx], name)) {
               return (e) idx;
            }
         }

         return NONE;
      }

   }

   Load::Load()
      : scenario_(scenario::NONE), turn_(0)
   {
   }

   namespace flag {

      namespace toggle {
         static const char *nametag[] =
         {
            "(none)",
            "tpt-plan",
            "transaction-per-minute",
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

         e code(const char *name)
         {
            for (unsigned idx = (unsigned)(NONE + 1); idx < (unsigned) END_OF_ENUM; ++idx) {
               if (xi::StrEqualNoCase(nametag[idx], name)) {
                  return (e) idx;
               }
            }

            return NONE;
         }

         bool default_value(e type)
         {
            switch (type) {
               default :
                  return false;
                  break;
            }
            return false;
         }
      }

      namespace number {
         static const char *nametag[] =
         {
            "(none)",
            "keep-connections",
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

         e code(const char *name)
         {
            for (unsigned idx = (unsigned)(NONE + 1); idx < (unsigned) END_OF_ENUM; ++idx) {
               if (xi::StrEqualNoCase(nametag[idx], name)) {
                  return (e) idx;
               }
            }

            return NONE;
         }

         int default_value(e type)
         {
            switch (type) {
               case KEEP_CONNECTIONS :
                  return 100;
               default :
                  return 200;
                  break;
            }

            return 200;
         }
      }

      namespace letter {
         static const char *nametag[] =
         {
            "(none)",
            "tpt-plan-file",
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

         e code(const char *name)
         {
            for (unsigned idx = (unsigned)(NONE + 1); idx < (unsigned) END_OF_ENUM; ++idx) {
               if (xi::StrEqualNoCase(nametag[idx], name)) {
                  return (e) idx;
               }
            }

            return NONE;
         }

         const char *default_value(e type)
         {
            switch (type) {
               default :
                  return "";
                  break;
            }

            return "";
         }
      }

   }

}
