// vim:ts=3:sts=3:sw=3

#ifndef TEMPLATE_MAIN_STATISTICS_H_
#define TEMPLATE_MAIN_STATISTICS_H_

#include <tbb/atomic.h>
#include "xi/singleton.hxx"
#include "xi/statistics.hxx"
#include "template/main/property.h"

namespace ih {


namespace stat {
   namespace protocol {
      typedef enum {
         NONE = 0,
         PUSH,
         END_OF_ENUM
      } e;
      const char *name(e type);
      bool        scope(e type);
      e           code(const char *name);
   }

   namespace service {
      typedef enum {
         NONE = 0,
         API01,
         API02,
         END_OF_ENUM
      } e;
      const char *name(e type);
      bool        scope(e type);
   }

   namespace debug {
      typedef enum {
         NONE = 0,
         INVALID_FSM,
         END_OF_ENUM
      } e;
      const char *name(e type);
      bool scope(e type);
   }
}


class Statistics : public xi::Singleton<Statistics>
{
   public :
      Statistics();
      ~Statistics();
      void Clear();

      void                 StatProtocol(bool is_recv, ih::stat::protocol::e protocol, int16_t status_code = 0);
      std::string          ShowProtocol();
      std::string          ShowProtocolHour();
      std::string          ShowProtocolRecent10Min();

      void                 StatService(bool is_recv, ih::stat::service::e service, int16_t status_code = 0);
      std::string          ShowService();
      std::string          ShowServiceHour();
      std::string          ShowServiceRecent10Min();

      void                 ResetDebug();
      void                 StatDebug(ih::stat::debug::e stat);
      std::string          ShowDebug();

   private :
      xi::Statistics      *protocol_stat_;
      xi::Statistics      *service_stat_;
      tbb::atomic<uint64_t> debug_stat_[ih::stat::debug::END_OF_ENUM];
};


}
#endif
