// vim:ts=3:sts=3:sw=3

#ifndef TEMPLATE_MAIN_CLI_H_
#define TEMPLATE_MAIN_CLI_H_ 
 
#include "xi/cli.hxx"
#include "xi/singleton.hxx"
#include "xi/string.hxx"

namespace ih {


namespace rept {
   typedef enum {
      NONE = 0,
      STAT_PROTOCOL,
      STAT_PROTOCOL_HOUR,
      STAT_PROTOCOL_RECENT_10MIN,
      STAT_SERVICE,
      STAT_SERVICE_HOUR,
      STAT_SERVICE_RECENT_10MIN,
      STAT_DEBUG,
      END_OF_ENUM
   } e;
   const char *tid_name(e param);
   const char *category_name(e param);
   bool        scope(e param);
   e           code(const char *tid, const char *category);
   bool        valid(e type);
}

class Cli : public xi::Singleton<Cli>, public xi::cli::Bridge
{
   public :
      Cli();
      virtual ~Cli();

      bool                 Initialize();
      std::string          HelpMessage() const;

      virtual bool         OnCommand(xi::cli::context_t ctx, xi::Tl1Command &tl1cmd);
      virtual std::string  OnShowInterval(xi::cli::context_t ctx, unsigned id);

      void                 SchedRept(xi::cli::context_t ctx, unsigned id, time_t interval);
      void                 CancRept(xi::cli::context_t ctx, unsigned id);

   private :
      std::string          Banner();
      std::string          ComposeHelpMessage();
      std::string          help_;
};


}
#endif
