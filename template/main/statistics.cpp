// vim:ts=3:sts=3:sw=3

#include "statistics.h"
#include <stdio.h>
#include "xi/logger.hxx"
#include "xi/util.hxx"

namespace ih {


namespace stat {
   namespace protocol {
      static const char *nametag[] =
      {
         "(none)",
         "PUSH",
         "end-of-enum"
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
         for (unsigned int index = NONE + 1; index < (unsigned) END_OF_ENUM; ++index) {
            if (xi::StrEqual(name, nametag[index]))
               return (e)index;
         }

         return NONE;
      }
   }

   namespace service {
      static const char *nametag[] =
      {
         "(none)",
         "API01",
         "API02",
         "end-of-enum"
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
   }

   namespace debug {
      static const char *nametag[] =
      {
         "(none)",
         "INVALID_FSM",
         "end-of-enum"
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
   }
}


Statistics::Statistics()
{
   protocol_stat_ = new xi::Statistics();
   for (uint16_t index = 1; index < (uint16_t)stat::protocol::END_OF_ENUM; ++index)
      protocol_stat_->SetMethodName(index, stat::protocol::name((stat::protocol::e)index));

   service_stat_ = new xi::Statistics();
   for (uint16_t index = 1; index < (uint16_t)stat::service::END_OF_ENUM; ++index)
      service_stat_->SetMethodName(index, stat::service::name((stat::service::e)index));

   ResetDebug();
}

Statistics::~Statistics()
{
   if (protocol_stat_) {
      delete protocol_stat_;
      protocol_stat_ = NULL;
   }

   if (service_stat_) {
      delete service_stat_;
      service_stat_ = NULL;
   }
}

void Statistics::Clear()
{
   protocol_stat_->Clear();
   service_stat_->Clear();
   ResetDebug();
}

void Statistics::StatProtocol(bool is_recv, ih::stat::protocol::e protocol, int16_t status_code)
{
   protocol_stat_->Stat(is_recv, protocol & 0xFFFF, status_code);
}

std::string Statistics::ShowProtocol()
{
   return protocol_stat_->ToString();
}

std::string Statistics::ShowProtocolHour()
{
   return protocol_stat_->ToStringHour();
}

std::string Statistics::ShowProtocolRecent10Min()
{
   return protocol_stat_->ToStringRecent10Min();
}

void Statistics::StatService(bool is_recv, ih::stat::service::e service, int16_t status_code)
{
   service_stat_->Stat(is_recv, service & 0xFFFF, status_code);
}

std::string Statistics::ShowService()
{
   return service_stat_->ToString();
}

std::string Statistics::ShowServiceHour()
{
   return service_stat_->ToStringHour();
}

std::string Statistics::ShowServiceRecent10Min()
{
   return service_stat_->ToStringRecent10Min();
}

void Statistics::ResetDebug()
{
   for (unsigned index = 0; index < (unsigned)stat::debug::END_OF_ENUM; ++index)
      debug_stat_[index] = 0;
}

void Statistics::StatDebug(ih::stat::debug::e stat)
{
   if (ih::stat::debug::scope(stat))
      debug_stat_[stat]++;
   else
      debug_stat_[ih::stat::debug::NONE]++;
}

std::string Statistics::ShowDebug()
{
   std::string output;

   xi::String line;
   for (unsigned index = 1; index < (unsigned)stat::debug::END_OF_ENUM; ++index)
   {
      line.Csnprintf(128, "    * debug-stat %-32s : %u\n", stat::debug::name((stat::debug::e)index), debug_stat_[index]);
      output += line.c_str();
   }

   return output;
}


}
