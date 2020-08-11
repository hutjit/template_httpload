// vim:ts=3:sts=3:sw=3

#include "cli.h"
#include <signal.h>
#include <stdio.h>
#include <openssl/crypto.h>
#include "xi/logger.hxx"
#include "xi/pretty_table.hxx"
#include "xi/util.hxx"
#include "interface_hub.h"
#include "property.h"
#include "statistics.h"
#include "rp_pool.h"
#include "load_generator.h"

namespace ih {

typedef std::string (*Tl1CliCallback)(IN xi::cli::context_t, xi::Tl1Command&);

typedef struct {
   const char *command;
   const char *tid;
   const char *desc;
   Tl1CliCallback callback;
   bool visible_help_menu;
} tl1cli_t;

std::string dummy_command(IN xi::cli::context_t ctx, IN xi::Tl1Command &tl1cmd);
std::string help(IN xi::cli::context_t ctx, IN xi::Tl1Command &tl1cmd);
std::string help_commands(IN xi::cli::context_t ctx, IN xi::Tl1Command &tl1cmd);
std::string rtrv_status_summary(IN xi::cli::context_t ctx, IN xi::Tl1Command &tl1cmd);
std::string chg_env(IN xi::cli::context_t ctx, IN xi::Tl1Command &tl1cmd);
std::string rtrv_env(IN xi::cli::context_t ctx, IN xi::Tl1Command &tl1cmd);
std::string sched_rept(IN xi::cli::context_t ctx, IN xi::Tl1Command &tl1cmd);
std::string canc_rept(IN xi::cli::context_t ctx, IN xi::Tl1Command &tl1cmd);
std::string rtrv_rept(IN xi::cli::context_t ctx, IN xi::Tl1Command &tl1cmd);
std::string clr_rept(IN xi::cli::context_t ctx, IN xi::Tl1Command &tl1cmd);
std::string rtrv_tst_cfg(IN xi::cli::context_t ctx, IN xi::Tl1Command &tl1cmd);
std::string init_tst_cfg(IN xi::cli::context_t ctx, IN xi::Tl1Command &tl1cmd);
std::string chg_tst_cfg(IN xi::cli::context_t ctx, IN xi::Tl1Command &tl1cmd);
std::string act_load_gen(IN xi::cli::context_t ctx, IN xi::Tl1Command &tl1cmd);
std::string chg_load_gen(IN xi::cli::context_t ctx, IN xi::Tl1Command &tl1cmd);
std::string canc_load_gen(IN xi::cli::context_t ctx, IN xi::Tl1Command &tl1cmd);
std::string rtrv_load_gen(IN xi::cli::context_t ctx, IN xi::Tl1Command &tl1cmd);
std::string rtrv_debug(IN xi::cli::context_t ctx, IN xi::Tl1Command &tl1cmd);

tl1cli_t cli_exec_table[] = {
   {
      "RTRV", NULL,
      "hidden",
      rtrv_status_summary, false
   },
   {
      "r", NULL,
      "hidden",
      rtrv_status_summary, false
   },
   {
      "CHG-ENV", NULL,
      "The change environment command is used to change the environment. LOG/TIMER",
      chg_env, true
   },
   {
      "CHG-ENV", "LOG",
      "The change environment(log) command is used to change the environment(log).",
      chg_env, false
   },
   {
      "CHG-ENV", "TIMER",
      "The change environment(timer) command is used to change the environment(timer).",
      chg_env, false
   },
   {
      "CHG-ENV", "TLS",
      "The change environment(tls) command is used to change the environment(tls).",
      chg_env, false
   },

   {
      "RTRV-ENV", NULL,
      "The following sections retrieve ENV information using the RTRV-ENV commands. LOG/TIMER",
      rtrv_env, true
   },
   {
      "RTRV-ENV", "LOG",
      "The following sections retrieve ENV:LOG information using the RTRV-ENV:LOG commands.",
      rtrv_env, false
   },
   {
      "RTRV-ENV", "TIMER",
      "The following sections retrieve ENV:TIMER information using the RTRV-ENV:TIMER commands.",
      rtrv_env, false
   },
   {
      "SCHED-REPT", NULL,
      "This command schedules statistics reporting. PROT/SVC",
      sched_rept, true
   },
   {
      "SCHED-REPT", "PROT",
      "This command schedules protocol statistics reporting.",
      sched_rept, false
   },
   {
      "SCHED-REPT", "SVC",
      "This command schedules service statistics reporting.",
      sched_rept, false
   },
   {
      "SCHED-REPT", "DEBUG",
      "This command schedules debug statistics reporting.",
      sched_rept, false
   },
   {
      "CANC-REPT", NULL,
      "Cancels the statistics reporting. PROT/SVC",
      canc_rept, true
   },
   {
      "CANC-REPT", "PROT",
      "Cancels the protocol statistics reporting.",
      canc_rept, false
   },
   {
      "CANC-REPT", "SVC",
      "Cancels the service statistics reporting.",
      canc_rept, false
   },
   {
      "CANC-REPT", "DEBUG",
      "Cancels the debug statistics reporting.",
      canc_rept, false
   },
   {
      "RTRV-REPT", NULL,
      "The following sections retrieve statistics using the RTRV-REPT:<PROT/SVC> commands.",
      rtrv_rept, true
   },
   {
      "RTRV-REPT", "PROT",
      "The following sections retrieve protocol statistics using the RTRV-REPT:PROT commands.",
      rtrv_rept, false
   },
   {
      "RTRV-REPT", "SVC",
      "The following sections retrieve service statistics using the RTRV-REPT:SVC commands.",
      rtrv_rept, false
   },
   {
      "RTRV-REPT", "DEBUG",
      "The following sections retrieve debug statistics using the RTRV-REPT:DEBUG commands.",
      rtrv_rept, false
   },
   {
      "CLR-REPT", NULL,
      "The clear statistics.",
      clr_rept, true
   },
   {
      "RTRV-TST-CFG", NULL,
      "The following sections retrieve test-config information using the RTRV-TST-CFG commands.",
      rtrv_tst_cfg, true
   },
   {
      "INIT-TST-CFG", NULL,
      "The initialize test-config command initializes the test-config. TOGGLE/NUMBER/LETTER",
      init_tst_cfg, true
   },
   {
      "CHG-TST-CFG", NULL,
      "The change test-config command is used to change the test-config. TOGGLE/NUMBER/LETTER",
      chg_tst_cfg, true
   },
   {
      "RTRV-TST-CFG", "TOGGLE",
      "The following sections retrieve test-config(boolean) information using the RTRV-TST-CFG:TOGGLE commands.",
      rtrv_tst_cfg, false
   },
   {
      "CHG-TST-CFG", "TOGGLE",
      "The change test-config(boolean) command is used to change the test-config(boolean).",
      chg_tst_cfg, false
   },
   {
      "INIT-TST-CFG", "TOGGLE",
      "The initialize test-config(boolean) command initializes the test-config(boolean).",
      init_tst_cfg, false
   },
   {
      "RTRV-TST-CFG", "NUMBER",
      "The following sections retrieve test-config(number) information using the RTRV-TST-CFG:NUMBER commands.",
      rtrv_tst_cfg, false
   },
   {
      "CHG-TST-CFG", "NUMBER",
      "The change test-config(number) command is used to change the test-config(number).",
      chg_tst_cfg, false
   },
   {
      "INIT-TST-CFG", "NUMBER",
      "The initialize test-config(number) command initializes the test-config(number).",
      init_tst_cfg, false
   },
   {
      "RTRV-TST-CFG", "LETTER",
      "The following sections retrieve test-config(string) information using the RTRV-TST-CFG:LETTER commands.",
      rtrv_tst_cfg, false
   },
   {
      "CHG-TST-CFG", "LETTER",
      "The change test-config(string) command is used to change the test-config(string).",
      chg_tst_cfg, false
   },
   {
      "INIT-TST-CFG", "LETTER",
      "The initialize test-config(string) command initializes the test-config(string).",
      init_tst_cfg, false
   },
   {
      "ACT-LOAD-GEN", NULL,
      "The activate load-generator command starts to load-script.",
      act_load_gen, true
   },
   {
      "ACT-LOAD-GEN", "ONCE",
      "The activate load-generator command starts to load-script.",
      act_load_gen, false
   },
   {
      "ACT-LOAD-GEN", "LONG",
      "The activate load-generator command starts to load-script.",
      act_load_gen, false
   },
   {
      "CHG-LOAD-GEN", "LONG",
      "The change load-generator command is used to change the long run load plan.",
      chg_load_gen, true
   },
   {
      "CANC-LOAD-GEN", NULL,
      "Cancels load-generator command stops to load-script.",
      canc_load_gen, true
   },
   {
      "RTRV-LOAD-GEN", NULL,
      "The following sections retrieve LOAD-GEN information using the ACT-LOAD-GEN/CHG-LOAD-GEN commands.",
      rtrv_load_gen, true
   },
   {
      "RTRV-DEBUG", NULL,
      "The following sections retrieve debugging information.",
      rtrv_debug, true
   },
   {
      "RTRV-DEBUG", "HTTP1-STATUS",
      "The following sections retrieve debugging information.",
      rtrv_debug, false
   },
   {
      "RTRV-DEBUG", "HTTP1-DEBUG-STATISTICS",
      "The following sections retrieve debugging information.",
      rtrv_debug, false
   },
   {
      "EXIT", NULL,
      "The exit command is used to exit from the CLI.",
      dummy_command, true
   },
   {
      "SHUTDOWN", NULL,
      "The shutdown command brings the process down.",
      dummy_command, true
   },
   {
      "HELP", NULL,
      "HELP[:COMMAND];\nex)\nHELP;\nHELP:RTRV-ENV;\n",
      help, true
   },
   { "HELP", "CHG-ENV", "hidden\n", help_commands, false },
   { "HELP", "RTRV-ENV", "hidden\n", help_commands, false },
   { "HELP", "SCHED-REPT", "hidden\n", help_commands, false },
   { "HELP", "CANC-REPT", "hidden\n", help_commands, false },
   { "HELP", "CLR-REPT", "hidden\n", help_commands, false },
   { "HELP", "RTRV-REPT", "hidden\n", help_commands, false },
   { "HELP", "ACT-LOAD-GEN", "hidden\n", help_commands, false },
   { "HELP", "CHG-LOAD-GEN", "hidden\n", help_commands, false },
   { "HELP", "CHG-TST-CFG", "hidden\n", help_commands, false },
   { "HELP", "INIT-TST-CFG", "hidden\n", help_commands, false },
   { "HELP", "RTRV-TST-CFG", "hidden\n", help_commands, false },
   { "HELP", "RTRV-DEBUG", "hidden\n", help_commands, false },
   {
      "h", NULL,
      "HELP[:COMMAND];\n",
      help, false
   }
};

namespace rept
{
   typedef struct {
      const char *tid;
      const char *category;
   } nametag_t;

   const nametag_t rept_table[] =
   {
      { "(none)", "(none)" },
      { "PROT", "FLOW" },
      { "PROT", "HOUR" },
      { "PROT", "RECENT10" },
      { "SVC", "FLOW" },
      { "SVC", "HOUR" },
      { "SVC", "RECENT10" },
      { "DEBUG", "TU" }
   };

   const char *tid_name(e type)
   {
      if (scope(type))
         return rept_table[type].tid;

      return "(critical-exception)";
   }

   const char *category_name(e type)
   {
      if (scope(type))
         return rept_table[type].category;

      return "(critical-exception)";
   }

   bool scope(e type)
   {
      if (END_OF_ENUM > (unsigned)type)
         return true;
      return false;
   }

   e code(const char *tid, const char *category)
   {
      for (unsigned idx = (unsigned)(NONE + 1); idx < (unsigned) END_OF_ENUM; ++idx) {
         if (xi::StrEqual(rept_table[idx].tid, tid) && xi::StrEqual(rept_table[idx].category, category))
            return (e) idx;
      }

      return NONE;
   }

   bool valid(e type)
   {
      if (type && (END_OF_ENUM > (unsigned)type))
         return true;
      return false;
   }
}

Cli::Cli()
{
}

Cli::~Cli()
{
}

bool Cli::Initialize()
{
   for (unsigned index = rept::STAT_PROTOCOL; index < rept::END_OF_ENUM; ++index) {
      char buffer[128];
      snprintf(buffer, sizeof(buffer), "SCHED-REPT:%s:CATEGORY=%s", rept::tid_name((rept::e)index), rept::category_name((rept::e)index));
      SetShowIntervalName(index, buffer);
   }
   help_ = ComposeHelpMessage();

   xi::cli::Parameters param;
   param.bind_addr_.SetIpPort("127.0.0.1", Property::Instance()->GetCliPort());
   param.banner_ = Banner();
   param.prompt_ = "cli> ";
   param.help_msg_ = help_;
   param.session_pool_size_ = 10;
   param.login_accounts_[Property::Instance()->GetCliID()] = Property::Instance()->GetCliPW();

   if (false == Bridge::Initialize(param))
      return false;

   return Start();
}

std::string Cli::HelpMessage() const
{
   return help_;
}

std::string Cli::Banner()
{
   xi::String buildtime;
   buildtime.Csnprintf(64, "%s %s", __DATE__, __TIME__);

   xi::String banner;
   banner.Csnprintf(1024,
         "  - %s\n"
         "  - %s\n"
         "\n"
         "  +-------------------------------------------+\n"
         "  | - version : %-29s |\n"
         "  | - date    : %-29s |\n"
         "  +-------------------------------------------+\n"
         "\n"
         , OpenSSL_version(OPENSSL_VERSION)
         , xi::Version()
         , Property::Instance()->Version()
         , buildtime.c_str()
         );

   return banner.c_log();
}

std::string Cli::ComposeHelpMessage()
{
   std::map<std::string, unsigned> cmdtable;
   for (unsigned index = 0; index < (sizeof(cli_exec_table) / sizeof(tl1cli_t)); ++index)
   {
      if (false == cli_exec_table[index].visible_help_menu)
         continue;

      std::string key = cli_exec_table[index].command;
      key += (xi::IsZero(cli_exec_table[index].tid) ? "" : cli_exec_table[index].tid);
      cmdtable[key] = index;
   }

   xi::PrettyTable table;
   table.SetMarginLeft(2);
   table.SetWidth(0, 20);
   table.SetWidth(1, 10);
   table.SetWidth(2, 42);
   table.SetHead(0, "COMMAND");
   table.SetHead(1, "TID");
   table.SetHead(2, "DESCRIPTION");


   uint32_t row_index = 0;
   for (auto &p : cmdtable)
   {
      table.SetColumnValue(row_index, 0, cli_exec_table[p.second].command);
      table.SetColumnValue(row_index, 1, (xi::IsZero(cli_exec_table[p.second].tid) ? "" : cli_exec_table[p.second].tid));
      table.SetColumnValue(row_index, 2, (xi::IsZero(cli_exec_table[p.second].desc) ? "" : cli_exec_table[p.second].desc));

      row_index++;
   }

   std::string showmsg = table.Print();
   showmsg +=
      "\n"
      "  a[:b[: ... c]];\n"
      "\n"
      "  where:\n"
      "  \"a\" is the command code.\n"
      "  \":b\" is the target identifier (TID).\n"
      "  \": ... c\" are other positions required for various commands.\n"
      "\n"
      ;

   return showmsg;
}

bool Cli::OnCommand(xi::cli::context_t ctx, xi::Tl1Command &tl1cmd)
{
   xi::String FN;
   FN.Csnprintf(128, "[CLI(%u)::OnCommand] ", ctx);

   xi::String showmsg;
   { // Command 
      xi::DateTime now;

      showmsg = "CMD[[32m[1m";
      showmsg += tl1cmd.Command();
      showmsg += "[0m] [36m";
      showmsg += now.ToString("%Y-%m-%d %H:%M:%S");
      showmsg += "[0m\n";

      if (false == ShowMessage(ctx, showmsg.c_str())) {
         WLOG(FN << "Send Command FAIL");
         return false;
      }
   }

   if (xi::StrEqualNoCase(tl1cmd.Command(), "EXIT"))
   {
      ShowMessage(ctx, "\nGoodbye~\n");
      return false;
   }
   else if (xi::StrEqual(tl1cmd.Command(), "SHUTDOWN"))
   {
      WLOG(FN << "shutdown");
      xi::Sleep(150);
      raise(SIGTERM);
      return true;
   }
   else if (xi::StrEqualNoCase(tl1cmd.Command(), "SHUTDOWN"))
   {
      ShowMessage(ctx, "Error: Command not found.\n");
      return ShowPrompt(ctx);
   }
   else if (xi::StrEqualNoCase(tl1cmd.Command(), "uptime"))
   {
      std::string uptime = Uptime();
      uptime += "\n";
      ShowMessage(ctx, uptime.c_str());
      return ShowPrompt(ctx);
   }

   showmsg.Clear();
   for (unsigned index = 0; index < (sizeof(cli_exec_table) / sizeof(tl1cli_t)); ++index)
   {
      if (xi::StrEqualNoCase(cli_exec_table[index].command, tl1cmd.Command()))
      {
         if (xi::StrEqualNoCase(cli_exec_table[index].tid, tl1cmd.Staging(0)))
         {
            showmsg = cli_exec_table[index].callback(ctx, tl1cmd);
            if ((false == showmsg.IsEmpty()) && (false == showmsg.IsEqual("DO_NOT_SHOW")))
               ShowMessage(ctx, showmsg.c_log());
            break;
         }
         else if (xi::IsZero(cli_exec_table[index].tid) && xi::IsZero(tl1cmd.Staging(0)))
         {
            showmsg = cli_exec_table[index].callback(ctx, tl1cmd);
            if ((false == showmsg.IsEmpty()) && (false == showmsg.IsEqual("DO_NOT_SHOW")))
               ShowMessage(ctx, showmsg.c_log());
            break;
         }
      }
   }

   if (showmsg.IsEqual("DO_NOT_SHOW"))
      return true;

   if (showmsg.IsEmpty())
      ShowMessage(ctx, "Error: Command not found.\n");

   return ShowPrompt(ctx);
}

std::string Cli::OnShowInterval(xi::cli::context_t ctx, unsigned id)
{
   DLOG("[CLI(" << ctx << ")::OnShowInterval] id:" << id);

   xi::String showmsg;

   xi::String load_status = ih::LoadGenerator::Instance()->GetStatus();
   showmsg.Csnprintf(1024*16,
         "\n"
         "  [[[33m[1m %s [0m]]\n"
         "\n"
         , load_status.c_log());

   switch ((rept::e)id)
   {
      case rept::STAT_PROTOCOL :
         showmsg += ih::Statistics::Instance()->ShowProtocol();
         break;
      case rept::STAT_PROTOCOL_HOUR :
         showmsg += ih::Statistics::Instance()->ShowProtocolHour();
         break;
      case rept::STAT_PROTOCOL_RECENT_10MIN :
         showmsg += ih::Statistics::Instance()->ShowProtocolRecent10Min();
         break;
      case rept::STAT_SERVICE :
         showmsg += ih::Statistics::Instance()->ShowService();
         break;
      case rept::STAT_SERVICE_HOUR :
         showmsg += ih::Statistics::Instance()->ShowServiceHour();
         break;
      case rept::STAT_SERVICE_RECENT_10MIN :
         showmsg += ih::Statistics::Instance()->ShowServiceRecent10Min();
         break;
      case rept::STAT_DEBUG :
         showmsg += ih::Statistics::Instance()->ShowDebug();
         break;
      default :
         showmsg = "unsupported feature";
         break;
   }

   return showmsg.c_log();
}

void Cli::SchedRept(xi::cli::context_t ctx, unsigned id, time_t interval)
{
   ScheduleShowInterval(ctx, id, interval);
}

void Cli::CancRept(xi::cli::context_t ctx, unsigned id)
{
   CancelShowInterval(ctx, id);
}

std::string dummy_command(IN xi::cli::context_t ctx, IN xi::Tl1Command &tl1cmd)
{
   if (0)
      DLOG("Purpose of removing waring: ctx[" << ctx << "] command[" << tl1cmd.Command() << "]");

   return "DO_NOT_SHOW";
}

std::string help(IN xi::cli::context_t ctx, IN xi::Tl1Command &tl1cmd)
{
   if (0)
      DLOG("Purpose of removing waring: ctx[" << ctx << "] command[" << tl1cmd.Command() << "]");

   return Cli::Instance()->HelpMessage();
}

std::string help_commands(IN xi::cli::context_t ctx, IN xi::Tl1Command &tl1cmd)
{
   if (0)
      DLOG("Purpose of removing waring: ctx[" << ctx << "] command[" << tl1cmd.Command() << "]");

   std::string showmsg;

   xi::PrettyTable table;
   table.SetMarginLeft(2);
   table.SetHead(0, "EXAMPLE");
   table.SetHead(1, "DESCRIPTION");

   uint32_t row_index = 0;
   char command[128];

   if (xi::StrEqualNoCase("CHG-ENV", tl1cmd.Staging(0)))
   {
      table.SetColumnValue(row_index, 0, "CHG-ENV:LOG:LEVEL=DEBUG;");
      table.SetColumnValue(row_index, 1, "DEBUG/INFO/WARNING/ERROR");
      ++row_index;

      table.SetColumnValue(row_index, 0, "CHG-ENV:LOG:LLT=ON;");
      table.SetColumnValue(row_index, 1, "ON/OFF");
      ++row_index;

      for (int idx = td::timer::NONE + 1; idx < td::timer::END_OF_ENUM; ++idx) {
         td::timer::e code = (td::timer::e) idx;
         snprintf(command, sizeof(command), "CHG-ENV:TIMER:%s=%u;"
               , td::timer::name(code)
               , Property::Instance()->GetTimerValue(code));

         table.SetColumnValue(row_index, 0, command);
         table.SetColumnValue(row_index, 1, "Millisecond");
         ++row_index;
      }

      showmsg = table.Print();
   }
   else if (xi::StrEqualNoCase("RTRV-ENV", tl1cmd.Staging(0)))
   {
      table.SetColumnValue(row_index, 0, "RTRV-ENV;");
      table.SetColumnValue(row_index, 1, "Retrieve all.");
      ++row_index;

      table.SetColumnValue(row_index, 0, "RTRV-ENV:LOG;");
      table.SetColumnValue(row_index, 1, "Retrieve log section.");
      ++row_index;

      table.SetColumnValue(row_index, 0, "RTRV-ENV:TIMER;");
      table.SetColumnValue(row_index, 1, "Retrieve timer section.");
      ++row_index;

      showmsg = table.Print();
   }
   else if (xi::StrEqualNoCase("SCHED-REPT", tl1cmd.Staging(0)))
   {
      table.SetColumnValue(row_index, 0, "SCHED-REPT:PROT:CATEGORY=FLOW,INTERVAL=5;");
      table.SetColumnValue(row_index, 1, "INTERVAL(second)");
      ++row_index;

      table.SetColumnValue(row_index, 0, "SCHED-REPT:PROT:CATEGORY=HOUR,INTERVAL=5;");
      ++row_index;

      table.SetColumnValue(row_index, 0, "SCHED-REPT:PROT:CATEGORY=RECENT10,INTERVAL=5;");
      ++row_index;

      table.SetColumnValue(row_index, 0, "SCHED-REPT:SVC:CATEGORY=FLOW,INTERVAL=5;");
      table.SetColumnValue(row_index, 1, "INTERVAL(second)");
      ++row_index;

      table.SetColumnValue(row_index, 0, "SCHED-REPT:SVC:CATEGORY=HOUR,INTERVAL=5;");
      ++row_index;

      table.SetColumnValue(row_index, 0, "SCHED-REPT:SVC:CATEGORY=RECENT10,INTERVAL=5;");
      ++row_index;

      table.SetColumnValue(row_index, 0, "SCHED-REPT:DEBUG:CATEGORY=TU,INTERVAL=5;");
      ++row_index;

      showmsg = table.Print();
   }
   else if (xi::StrEqualNoCase("CANC-REPT", tl1cmd.Staging(0)))
   {
      table.SetColumnValue(row_index, 0, "CANC-REPT:PROT:CATEGORY=FLOW;");
      ++row_index;

      table.SetColumnValue(row_index, 0, "CANC-REPT:PROT:CATEGORY=HOUR;");
      ++row_index;

      table.SetColumnValue(row_index, 0, "CANC-REPT:PROT:CATEGORY=RECENT10;");
      ++row_index;

      table.SetColumnValue(row_index, 0, "CANC-REPT:SVC:CATEGORY=FLOW;");
      ++row_index;

      table.SetColumnValue(row_index, 0, "CANC-REPT:SVC:CATEGORY=HOUR;");
      ++row_index;

      table.SetColumnValue(row_index, 0, "CANC-REPT:SVC:CATEGORY=RECENT10;");
      ++row_index;

      table.SetColumnValue(row_index, 0, "CANC-REPT:DEBUG:CATEGORY=TU;");
      ++row_index;

      showmsg = table.Print();
   }
   else if (xi::StrEqualNoCase("CLR-REPT", tl1cmd.Staging(0)))
   {
      table.SetColumnValue(row_index, 0, "CLR-REPT;");
      table.SetColumnValue(row_index, 1, "Clear statistics.");
      ++row_index;

      showmsg = table.Print();
   }
   else if (xi::StrEqualNoCase("RTRV-REPT", tl1cmd.Staging(0)))
   {
      table.SetColumnValue(row_index, 0, "RTRV-REPT:PROT:CATEGORY=FLOW;");
      ++row_index;

      table.SetColumnValue(row_index, 0, "RTRV-REPT:PROT:CATEGORY=HOUR;");
      ++row_index;

      table.SetColumnValue(row_index, 0, "RTRV-REPT:PROT:CATEGORY=RECENT10;");
      ++row_index;

      table.SetColumnValue(row_index, 0, "RTRV-REPT:SVC:CATEGORY=FLOW;");
      ++row_index;

      table.SetColumnValue(row_index, 0, "RTRV-REPT:SVC:CATEGORY=HOUR;");
      ++row_index;

      table.SetColumnValue(row_index, 0, "RTRV-REPT:SVC:CATEGORY=RECENT10;");
      ++row_index;

      table.SetColumnValue(row_index, 0, "RTRV-REPT:DEBUG:CATEGORY=TU;");
      ++row_index;

      showmsg = table.Print();
   }
   else if (xi::StrEqualNoCase("ACT-LOAD-GEN", tl1cmd.Staging(0)))
   {
      // long-run
      table.SetColumnValue(row_index, 0, "ACT-LOAD-GEN:LONG:TPS=10;");
      table.SetColumnValue(row_index, 1, "Perform a performance test unlimited.");
      ++row_index;

      table.SetColumnValue(row_index, 0, "ACT-LOAD-GEN:LONG:TPS=10,DURATION=60;");
      table.SetColumnValue(row_index, 1, "Perform a performance test for 1 minute.");
      ++row_index;

      // once
      for (unsigned index = (unsigned)test::scenario::NONE + 1; index < (unsigned)test::scenario::END_OF_ENUM; ++index) {
         snprintf(command, sizeof(command), "ACT-LOAD-GEN:ONCE:SCRIPT=%s;", test::scenario::name((test::scenario::e) index));
         table.SetColumnValue(row_index, 0, command);
         ++row_index;
      }

      showmsg = table.Print();
   }
   else if (xi::StrEqualNoCase("CHG-LOAD-GEN", tl1cmd.Staging(0)))
   {
      xi::String script_list(1024);
      script_list += "<SCRIPT>\n";
      for (unsigned index = (unsigned)test::scenario::NONE + 1; index < (unsigned)test::scenario::END_OF_ENUM; ++index) {
         script_list += "  - ";
         script_list += test::scenario::name((test::scenario::e) index);
         script_list += "\n";
      }

      table.SetColumnValue(row_index, 0, "CHG-LOAD-GEN:LONG:PLAN=\"<SCRIPT> <TPS>[,<SCRIPT> <TPS>]\";");
      table.SetColumnValue(row_index, 1, script_list.c_log());
      ++row_index;

      showmsg = table.Print();
   }
   else if (xi::StrEqualNoCase("CHG-TST-CFG", tl1cmd.Staging(0)))
   {
      for (unsigned idx = (unsigned)test::flag::toggle::NONE + 1; idx < (unsigned)test::flag::toggle::END_OF_ENUM; ++idx)
      {
         test::flag::toggle::e code = (test::flag::toggle::e) idx;
         snprintf(command, sizeof(command), "CHG-TST-CFG:TOGGLE:%s=ON;", test::flag::toggle::name(code));

         table.SetColumnValue(row_index, 0, command);
         table.SetColumnValue(row_index, 1, "ON/OFF");
         ++row_index;
      }

      for (unsigned idx = (unsigned)test::flag::number::NONE + 1; idx < (unsigned)test::flag::number::END_OF_ENUM; ++idx)
      {
         test::flag::number::e code = (test::flag::number::e) idx;
         snprintf(command, sizeof(command), "CHG-TST-CFG:NUMBER:%s=200;", test::flag::number::name(code));

         table.SetColumnValue(row_index, 0, command);
         table.SetColumnValue(row_index, 1, "Number value");
         ++row_index;
      }

      for (unsigned idx = (unsigned)test::flag::letter::NONE + 1; idx < (unsigned)test::flag::letter::END_OF_ENUM; ++idx)
      {
         test::flag::letter::e code = (test::flag::letter::e) idx;
         snprintf(command, sizeof(command), "CHG-TST-CFG:LETTER:%s=\"<string>\";", test::flag::letter::name(code));

         table.SetColumnValue(row_index, 0, command);
         table.SetColumnValue(row_index, 1, "String value");
         ++row_index;
      }

      showmsg = table.Print();
   }
   else if (xi::StrEqualNoCase("INIT-TST-CFG", tl1cmd.Staging(0)))
   {
      table.SetColumnValue(row_index, 0, "INIT-TST-CFG;");
      table.SetColumnValue(row_index, 1, "Initialize all.");
      ++row_index;

      table.SetColumnValue(row_index, 0, "INIT-TST-CFG:TOGGLE;");
      table.SetColumnValue(row_index, 1, "Initialize toggle section.");
      ++row_index;

      table.SetColumnValue(row_index, 0, "INIT-TST-CFG:TOGGLE:<ITEM>[,<ITEM>];");
      table.SetColumnValue(row_index, 1, "Initialize specific items.");
      ++row_index;

      table.SetColumnValue(row_index, 0, "INIT-TST-CFG:NUMBER;");
      table.SetColumnValue(row_index, 1, "Initialize number section.");
      ++row_index;

      table.SetColumnValue(row_index, 0, "INIT-TST-CFG:NUMBER:<ITEM>[,<ITEM>];");
      table.SetColumnValue(row_index, 1, "Initialize specific items.");
      ++row_index;

      table.SetColumnValue(row_index, 0, "INIT-TST-CFG:LETTER;");
      table.SetColumnValue(row_index, 1, "Initialize letter section.");
      ++row_index;

      table.SetColumnValue(row_index, 0, "INIT-TST-CFG:LETTER:<ITEM>[,<ITEM>];");
      table.SetColumnValue(row_index, 1, "Initialize specific items.");
      ++row_index;

      showmsg = table.Print();
   }
   else if (xi::StrEqualNoCase("RTRV-TST-CFG", tl1cmd.Staging(0)))
   {
      table.SetColumnValue(row_index, 0, "RTRV-TST-CFG;");
      table.SetColumnValue(row_index, 1, "Retrieve all.");
      ++row_index;

      table.SetColumnValue(row_index, 0, "RTRV-TST-CFG:TOGGLE;");
      table.SetColumnValue(row_index, 1, "Retrieve toggle section.");
      ++row_index;

      table.SetColumnValue(row_index, 0, "RTRV-TST-CFG:NUMBER;");
      table.SetColumnValue(row_index, 1, "Retrieve number section.");
      ++row_index;

      table.SetColumnValue(row_index, 0, "RTRV-TST-CFG:LETTER;");
      table.SetColumnValue(row_index, 1, "Retrieve letter section.");
      ++row_index;

      showmsg = table.Print();
   }
   else if (xi::StrEqualNoCase("RTRV-DEBUG", tl1cmd.Staging(0)))
   {
      table.SetColumnValue(row_index, 0, "RTRV-DEBUG;");
      table.SetColumnValue(row_index, 1, "Retrieve all.");
      ++row_index;

      table.SetColumnValue(row_index, 0, "RTRV-DEBUG:HTTP1-STATUS;");
      table.SetColumnValue(row_index, 1, "Retrieve http1 status.");
      ++row_index;

      table.SetColumnValue(row_index, 0, "RTRV-DEBUG:HTTP1-DEBUG-STATISTICS;");
      table.SetColumnValue(row_index, 1, "Retrieve http1 debug-statistics.");
      ++row_index;

      showmsg = table.Print();
   }

   return showmsg;
}

std::string rtrv_status_summary(IN xi::cli::context_t ctx, IN xi::Tl1Command &tl1cmd)
{
   if (0)
      DLOG("Purpose of removing waring: ctx[" << ctx << "] command[" << tl1cmd.Command() << "]");

   std::string showmsg;

   showmsg += "  * PLAN    : ";
   showmsg += ih::Property::Instance()->ToStringLoadScenario();
   showmsg += "\n";
   showmsg += "  * STATUS  : [1;33m";
   showmsg += ih::LoadGenerator::Instance()->GetStatus();
   showmsg += "[0m\n";
   showmsg += "\n";
   showmsg += RpPool::Instance()->Status();

   return showmsg;
}

std::string chg_env(IN xi::cli::context_t ctx, IN xi::Tl1Command &tl1cmd)
{
   if (0)
      DLOG("Purpose of removing waring: ctx[" << ctx << "] command[" << tl1cmd.Command() << "]");

   std::string showmsg;
   char conv[128];

   if (xi::StrEqualNoCase("LOG", tl1cmd.Staging(0)))
   {
      if (tl1cmd.Data("LEVEL"))
      {
         xi::loglevel::e level = xi::loglevel::code(tl1cmd.Data("LEVEL"));
         if (xi::loglevel::NONE != level)
         {
            snprintf(conv, sizeof(conv), "* change ENV:LOG - level (%s -> %s) success\n"
                  , xi::loglevel::name(GETLOGLEVEL())
                  , xi::loglevel::name(level));
            Property::Instance()->SetLogLevel(level);

            showmsg += conv;
         }
      }

      if (tl1cmd.Data("LLT"))
      {
         if (xi::StrEqual(tl1cmd.Data("LLT"), "ON"))
         {
            snprintf(conv, sizeof(conv), "* change ENV:LOG - llt (%s -> %s) success\n"
                  , (CANLLTLOGGING() ? "ON" : "OFF")
                  , tl1cmd.Data("LLT"));
            SETLLTFLAG(true);

            showmsg += conv;
         }
         else if (xi::StrEqual(tl1cmd.Data("LLT"), "OFF"))
         {
            snprintf(conv, sizeof(conv), "* change ENV:LOG - llt (%s -> %s) success\n"
                  , (CANLLTLOGGING() ? "ON" : "OFF")
                  , tl1cmd.Data("LLT"));
            SETLLTFLAG(false);

            showmsg += conv;
         }
      }
   }
   else if (xi::StrEqualNoCase("TIMER", tl1cmd.Staging(0)))
   {
      for (uint32_t idx = 0; idx < tl1cmd.DataBlockSize(); ++idx)
      {
         td::timer::e code = td::timer::code(tl1cmd.DataName(idx));
         if (td::timer::NONE == code)
            continue;

         xi::String value = tl1cmd.DataValue(idx);
         if (value.AsUInt32())
         {
            snprintf(conv, sizeof(conv), "* change ENV:TIMER - %s (%u -> %u) success\n"
                  , td::timer::name(code)
                  , Property::Instance()->GetTimerValue(code)
                  , value.AsUInt32());
            Property::Instance()->SetTimerValue(code, value.AsUInt32());
            showmsg += conv;
         }
      }
   }
   else if (xi::StrEqualNoCase("TLS", tl1cmd.Staging(0)))
   {
      if (tl1cmd.Data("CERT"))
      {
         showmsg += "* change ENV:TLS - certificate (";
         showmsg += Property::Instance()->GetHttpServerCertificate();
         showmsg += " -> ";
         showmsg += tl1cmd.Data("CERT");
         showmsg += ") success\n";

         Property::Instance()->SetHttpServerCertificate(tl1cmd.Data("CERT"));
      }

      if (tl1cmd.Data("KEY"))
      {
         showmsg += "* change ENV:TLS - key (";
         showmsg += Property::Instance()->GetHttpServerCertKey();
         showmsg += " -> ";
         showmsg += tl1cmd.Data("KEY");
         showmsg += ") success\n";

         Property::Instance()->SetHttpServerCertKey(tl1cmd.Data("KEY"));
      }

      std::vector<std::string> param;
      param[0] = "http2-reload-server-certificate";
      param[1] = Property::Instance()->GetHttpServerCertificate();
      param[1] = Property::Instance()->GetHttpServerCertKey();
      if (false == InterfaceHub::Instance()->ChangeResource(param))
      {
         showmsg += "\n";
         showmsg += param[0];
         showmsg += " fail\n";
      }
      else
      {
         showmsg += "\n";
         showmsg += param[0];
         showmsg += " success\n";
      }
   }

   return showmsg;
}

std::string rtrv_env(IN xi::cli::context_t ctx, IN xi::Tl1Command &tl1cmd)
{
   if (0)
      DLOG("Purpose of removing waring: ctx[" << ctx << "] command[" << tl1cmd.Command() << "]");

   uint32_t row_index = 0;
   xi::PrettyTable table;
   table.SetMarginLeft(2);
   table.SetHead(0, "SECTION");
   table.SetHead(1, "ITEM");
   table.SetHead(2, "VALUE");

   if (xi::IsZero(tl1cmd.Staging(0)) || xi::StrEqualNoCase("LOG", tl1cmd.Staging(0)))
   {
      table.SetColumnValue(row_index, 0, "ENV:LOG");
      table.SetColumnValue(row_index, 1, "LEVEL");
      table.SetColumnValue(row_index, 2, xi::loglevel::name(GETLOGLEVEL()));
      row_index++;

      table.SetColumnValue(row_index, 0, "ENV:LOG");
      table.SetColumnValue(row_index, 1, "LLT");
      table.SetColumnValue(row_index, 2, (CANLLTLOGGING() ? "ON" : "OFF"));
      row_index++;
   }

   if (xi::IsZero(tl1cmd.Staging(0)) || xi::StrEqualNoCase("TIMER", tl1cmd.Staging(0)))
   {
      for (int idx = td::timer::NONE + 1; idx < td::timer::END_OF_ENUM; ++idx) {
         table.SetColumnValue(row_index, 0, "ENV:TIMER");
         table.SetColumnValue(row_index, 1, td::timer::name((td::timer::e)idx));
         table.SetColumnValue(row_index, 2, (uint64_t) Property::Instance()->GetTimerValue((td::timer::e)idx));
         row_index++;
      }
   }

   if (xi::IsZero(tl1cmd.Staging(0)) || xi::StrEqualNoCase("TLS", tl1cmd.Staging(0)))
   {
      table.SetColumnValue(row_index, 0, "ENV:TLS");
      table.SetColumnValue(row_index, 1, "CERT");
      table.SetColumnValue(row_index, 2, Property::Instance()->GetHttpServerCertificate());
      row_index++;

      table.SetColumnValue(row_index, 0, "ENV:TLS");
      table.SetColumnValue(row_index, 1, "KEY");
      table.SetColumnValue(row_index, 2, Property::Instance()->GetHttpServerCertKey());
      row_index++;
   }

   return table.Print();
}

std::string sched_rept(IN xi::cli::context_t ctx, IN xi::Tl1Command &tl1cmd)
{
   std::string showmsg;

   rept::e code = rept::code(tl1cmd.Staging(0), tl1cmd.Data("CATEGORY"));
   if (rept::NONE != code) {
      time_t interval = atoi(xi::IsZero(tl1cmd.Data("INTERVAL")) ? "60" : tl1cmd.Data("INTERVAL"));

      if (Cli::Instance()->ScheduleShowInterval(ctx, code, interval))
         showmsg = "success\n";
      else
         showmsg = "fail\n";
   }

   return showmsg;
}

std::string canc_rept(IN xi::cli::context_t ctx, IN xi::Tl1Command &tl1cmd)
{
   std::string showmsg;

   rept::e code = rept::code(tl1cmd.Staging(0), tl1cmd.Data("CATEGORY"));
   if (rept::NONE != code) {
      if (Cli::Instance()->CancelShowInterval(ctx, code))
         showmsg = "success\n";
      else
         showmsg = "fail\n";
   }

   return showmsg;
}

std::string rtrv_rept(IN xi::cli::context_t ctx, IN xi::Tl1Command &tl1cmd)
{
   if (0)
      DLOG("Purpose of removing waring: ctx[" << ctx << "] command[" << tl1cmd.Command() << "]");

   std::string showmsg;

   rept::e code = rept::code(tl1cmd.Staging(0), tl1cmd.Data("CATEGORY"));
   switch (code) {
      case rept::STAT_PROTOCOL :
         showmsg = ih::Statistics::Instance()->ShowProtocol();
         break;

      case rept::STAT_PROTOCOL_HOUR :
         showmsg = ih::Statistics::Instance()->ShowProtocolHour();
         break;

      case rept::STAT_PROTOCOL_RECENT_10MIN :
         showmsg = ih::Statistics::Instance()->ShowProtocolRecent10Min();
         break;

      case rept::STAT_SERVICE :
         showmsg = ih::Statistics::Instance()->ShowService();
         break;

      case rept::STAT_SERVICE_HOUR :
         showmsg = ih::Statistics::Instance()->ShowServiceHour();
         break;

      case rept::STAT_SERVICE_RECENT_10MIN :
         showmsg = ih::Statistics::Instance()->ShowServiceRecent10Min();
         break;

      case rept::STAT_DEBUG :
         showmsg = ih::Statistics::Instance()->ShowDebug();
         break;

      default :
         break;
   }

   return showmsg;
}

std::string clr_rept(IN xi::cli::context_t ctx, IN xi::Tl1Command &tl1cmd)
{
   if (0)
      DLOG("Purpose of removing waring: ctx[" << ctx << "] command[" << tl1cmd.Command() << "]");

   ih::Statistics::Instance()->Clear();

   return "success.\n";
}

std::string rtrv_tst_cfg(IN xi::cli::context_t ctx, IN xi::Tl1Command &tl1cmd)
{
   Property *property = Property::Instance();

   if (0)
      DLOG("Purpose of removing waring: ctx[" << ctx << "] command[" << tl1cmd.Command() << "]");

   xi::PrettyTable table;
   table.SetMarginLeft(2);
   table.SetHead(0, "SECTION");
   table.SetHead(1, "ITEM");
   table.SetHead(2, "VALUE");

   char value[128];
   uint32_t row_index = 0;

   if (xi::StrEqualNoCase("TOGGLE", tl1cmd.Staging(0)))
   {
      for (unsigned idx = (unsigned)test::flag::toggle::NONE + 1; idx < (unsigned)test::flag::toggle::END_OF_ENUM; ++idx)
      {
         test::flag::toggle::e code = (test::flag::toggle::e) idx;
         snprintf(value, sizeof(value), "%s%s", (property->GetTestFlag(code) ? "ON" : "OFF"), (property->SameAsDefault(code) ? "" : " *"));

         table.SetColumnValue(row_index, 0, "TST-CFG:TOGGLE");
         table.SetColumnValue(row_index, 1, test::flag::toggle::name(code));
         table.SetColumnValue(row_index, 2, value);

         row_index++;
      }
   }
   else if (xi::StrEqualNoCase("NUMBER", tl1cmd.Staging(0)))
   {
      for (unsigned idx = (unsigned)test::flag::number::NONE + 1; idx < (unsigned)test::flag::number::END_OF_ENUM; ++idx)
      {
         test::flag::number::e code = (test::flag::number::e) idx;
         snprintf(value, sizeof(value), "%d%s", property->GetTestFlag(code), (property->SameAsDefault(code) ? "" : " *"));

         table.SetColumnValue(row_index, 0, "TST-CFG:NUMBER");
         table.SetColumnValue(row_index, 1, test::flag::number::name(code));
         table.SetColumnValue(row_index, 2, value);

         row_index++;
      }
   }
   else if (xi::StrEqualNoCase("LETTER", tl1cmd.Staging(0)))
   {
      for (unsigned idx = (unsigned)test::flag::letter::NONE + 1; idx < (unsigned)test::flag::letter::END_OF_ENUM; ++idx)
      {
         test::flag::letter::e code = (test::flag::letter::e) idx;
         snprintf(value, sizeof(value), "%s%s"
               , (xi::IsZero(property->GetTestFlag(code)) ? "(null)" : property->GetTestFlag(code))
               , (property->SameAsDefault(code) ? "" : " *"));

         table.SetColumnValue(row_index, 0, "TST-CFG:LETTER");
         table.SetColumnValue(row_index, 1, test::flag::letter::name(code));
         table.SetColumnValue(row_index, 2, value);

         row_index++;
      }
   }
   else
   {
      for (unsigned idx = (unsigned)test::flag::toggle::NONE + 1; idx < (unsigned)test::flag::toggle::END_OF_ENUM; ++idx)
      {
         test::flag::toggle::e code = (test::flag::toggle::e) idx;
         snprintf(value, sizeof(value), "%s%s", (property->GetTestFlag(code) ? "ON" : "OFF"), (property->SameAsDefault(code) ? "" : " *"));

         table.SetColumnValue(row_index, 0, "TST-CFG:TOGGLE");
         table.SetColumnValue(row_index, 1, test::flag::toggle::name(code));
         table.SetColumnValue(row_index, 2, value);

         row_index++;
      }

      for (unsigned idx = (unsigned)test::flag::number::NONE + 1; idx < (unsigned)test::flag::number::END_OF_ENUM; ++idx)
      {
         test::flag::number::e code = (test::flag::number::e) idx;
         snprintf(value, sizeof(value), "%d%s", property->GetTestFlag(code), (property->SameAsDefault(code) ? "" : " *"));

         table.SetColumnValue(row_index, 0, "TST-CFG:NUMBER");
         table.SetColumnValue(row_index, 1, test::flag::number::name(code));
         table.SetColumnValue(row_index, 2, value);

         row_index++;
      }

      for (unsigned idx = (unsigned)test::flag::letter::NONE + 1; idx < (unsigned)test::flag::letter::END_OF_ENUM; ++idx)
      {
         test::flag::letter::e code = (test::flag::letter::e) idx;
         snprintf(value, sizeof(value), "%s%s"
               , (xi::IsZero(property->GetTestFlag(code)) ? "(null)" : property->GetTestFlag(code))
               , (property->SameAsDefault(code) ? "" : " *"));

         table.SetColumnValue(row_index, 0, "TST-CFG:LETTER");
         table.SetColumnValue(row_index, 1, test::flag::letter::name(code));
         table.SetColumnValue(row_index, 2, value);

         row_index++;
      }
   }

   return table.Print();
}

std::string init_tst_cfg(IN xi::cli::context_t ctx, IN xi::Tl1Command &tl1cmd)
{
   Property *property = Property::Instance();

   if (0)
      DLOG("Purpose of removing waring: ctx[" << ctx << "] command[" << tl1cmd.Command() << "]");

   std::string showmsg;

   if (xi::StrEqualNoCase("TOGGLE", tl1cmd.Staging(0)))
   {
      if (tl1cmd.DataBlockSize())
      {
         for (uint32_t idx = 0; idx < tl1cmd.DataBlockSize(); ++idx)
         {
            test::flag::toggle::e code = test::flag::toggle::code(tl1cmd.DataName(idx));
            if (test::flag::toggle::NONE == code)
               continue;
            property->SetTestFlag(code, test::flag::toggle::default_value(code));
            showmsg += "TST-CFG:TOGGLE - ";
            showmsg += test::flag::toggle::name(code);
            showmsg +=  " initialized.\n";
         }
      }
      else
      {
         for (unsigned idx = (unsigned)test::flag::toggle::NONE + 1; idx < (unsigned)test::flag::toggle::END_OF_ENUM; ++idx)
            property->SetTestFlag((test::flag::toggle::e)idx, test::flag::toggle::default_value((test::flag::toggle::e)idx));
         showmsg = "TST-CFG:TOGGLE initialized.\n";
      }
   }
   else if (xi::StrEqualNoCase("NUMBER", tl1cmd.Staging(0)))
   {
      if (tl1cmd.DataBlockSize())
      {
         for (uint32_t idx = 0; idx < tl1cmd.DataBlockSize(); ++idx)
         {
            test::flag::number::e code = test::flag::number::code(tl1cmd.DataName(idx));
            if (test::flag::number::NONE == code)
               continue;
            property->SetTestFlag(code, test::flag::number::default_value(code));
            showmsg += "TST-CFG:NUMBER - ";
            showmsg += test::flag::number::name(code);
            showmsg +=  " initialized.\n";
         }
      }
      else
      {
         for (unsigned idx = (unsigned)test::flag::number::NONE + 1; idx < (unsigned)test::flag::number::END_OF_ENUM; ++idx)
            property->SetTestFlag((test::flag::number::e)idx, test::flag::number::default_value((test::flag::number::e)idx));
         showmsg = "TST-CFG:NUMBER initialized.\n";
      }
   }
   else if (xi::StrEqualNoCase("LETTER", tl1cmd.Staging(0)))
   {
      if (tl1cmd.DataBlockSize())
      {
         for (uint32_t idx = 0; idx < tl1cmd.DataBlockSize(); ++idx)
         {
            test::flag::letter::e code = test::flag::letter::code(tl1cmd.DataName(idx));
            if (test::flag::letter::NONE == code)
               continue;
            property->SetTestFlag(code, test::flag::letter::default_value(code));
            showmsg += "TST-CFG:LETTER - ";
            showmsg += test::flag::letter::name(code);
            showmsg +=  " initialized.\n";
         }
      }
      else
      {
         for (unsigned idx = (unsigned)test::flag::letter::NONE + 1; idx < (unsigned)test::flag::letter::END_OF_ENUM; ++idx)
            property->SetTestFlag((test::flag::letter::e)idx, test::flag::letter::default_value((test::flag::letter::e)idx));
         showmsg = "TST-CFG:LETTER initialized.\n";
      }
   }

   return showmsg;
}

std::string chg_tst_cfg(IN xi::cli::context_t ctx, IN xi::Tl1Command &tl1cmd)
{
   Property *property = Property::Instance();

   if (0)
      DLOG("Purpose of removing waring: ctx[" << ctx << "] command[" << tl1cmd.Command() << "]");

   std::string showmsg;

   if (xi::StrEqualNoCase("TOGGLE", tl1cmd.Staging(0)))
   {
      xi::String conv;
      for (uint32_t idx = 0; idx < tl1cmd.DataBlockSize(); ++idx)
      {
         test::flag::toggle::e code = test::flag::toggle::code(tl1cmd.DataName(idx));
         if (test::flag::toggle::NONE == code)
            continue;

         xi::String value = tl1cmd.DataValue(idx);
         value.ToUpper();

         if (value.IsEqual("ON")) {
         } else if (value.IsEqual("OFF")) {
         } else {
            continue;
         }

         conv.Csnprintf(256, "* change %s (%s -> %s) success\n"
               , test::flag::toggle::name(code)
               , (Property::Instance()->GetTestFlag(code) ? "ON" : "OFF")
               , value.c_str());
         Property::Instance()->SetTestFlag(code, (value.IsEqual("ON") ? true : false));
         showmsg += conv.c_str();
      }
   }
   else if (xi::StrEqualNoCase("NUMBER", tl1cmd.Staging(0)))
   {
      xi::String conv;
      for (uint32_t idx = 0; idx < tl1cmd.DataBlockSize(); ++idx)
      {
         test::flag::number::e code = test::flag::number::code(tl1cmd.DataName(idx));
         if (test::flag::number::NONE == code)
            continue;

         xi::String value = tl1cmd.DataValue(idx);

         conv.Csnprintf(256, "* change TST-CFG:NUMBER - %s (%d -> %d) success\n"
               , test::flag::number::name((test::flag::number::e)code)
               , property->GetTestFlag((test::flag::number::e)code)
               , value.AsInt());
         property->SetTestFlag((test::flag::number::e)code, value.AsInt());
         showmsg += conv.c_str();
      }
   }
   else if (xi::StrEqualNoCase("LETTER", tl1cmd.Staging(0)))
   {
      xi::String conv;
      for (uint32_t idx = 0; idx < tl1cmd.DataBlockSize(); ++idx)
      {
         test::flag::letter::e code = test::flag::letter::code(tl1cmd.DataName(idx));
         if (test::flag::letter::NONE == code)
            continue;

         xi::String value = tl1cmd.DataValue(idx);
         conv.Csnprintf(256, "* change TST-CFG:LETTER - %s (%s -> %s) success\n"
               , test::flag::letter::name((test::flag::letter::e)code)
               , (xi::IsZero(property->GetTestFlag((test::flag::letter::e)code)) ? "(null)" : property->GetTestFlag((test::flag::letter::e)code))
               , value.c_log());
         property->SetTestFlag((test::flag::letter::e)code, value.c_str());
         showmsg += conv.c_str();
      }
   }

   return showmsg;
}

std::string act_load_gen(IN xi::cli::context_t ctx, IN xi::Tl1Command &tl1cmd)
{
   if (0)
      DLOG("Purpose of removing waring: ctx[" << ctx << "] command[" << tl1cmd.Command() << "]");

   std::string showmsg;

   ih::Property::Instance()->SetAddrLimit(ih::Property::Instance()->GetTestFlag(test::flag::number::KEEP_CONNECTIONS));

   if (xi::StrEqualNoCase("ONCE", tl1cmd.Staging(0)))
   {
      if (tl1cmd.Data("SCRIPT"))
      {
         test::scenario::e scenario = test::scenario::code(tl1cmd.Data("SCRIPT"));
         if (test::scenario::NONE != scenario)
         {
            if (LoadGenerator::Instance()->Command(td::loadcmd::UNIT_TEST, scenario, 1, ""))
               showmsg = "success.\n";
            else
               showmsg = "fail.\n";
         }
      }
   }
   else if (xi::StrEqualNoCase("LONG", tl1cmd.Staging(0)))
   {
      if (tl1cmd.Data("TPS"))
      {
         unsigned cps = atoi(tl1cmd.Data("TPS"));
         std::string option = (tl1cmd.Data("DURATION") ? tl1cmd.Data("DURATION") : "");

         if (0 == cps)
         {
            showmsg = "invalid <cps>\n";
         }
         else
         {
            if (ih::LoadGenerator::Instance()->Command(td::loadcmd::START, test::scenario::NONE, cps, option))
               showmsg = "success\n";
            else
               showmsg = "fail\n";
         }
      }
   }

   return showmsg;
}

std::string chg_load_gen(IN xi::cli::context_t ctx, IN xi::Tl1Command &tl1cmd)
{
   if (0)
      DLOG("Purpose of removing waring: ctx[" << ctx << "] command[" << tl1cmd.Command() << "]");

   std::string showmsg;

   if (tl1cmd.Data("PLAN")) {
      xi::String plan = tl1cmd.Data("PLAN");
      std::list<test::Load> loadlist = ih::Property::Instance()->ParseLoadScenario(plan);
      if (false == loadlist.empty()) {
         showmsg = "success.\n";
         ih::Property::Instance()->SetLoadScenario(loadlist);
      }
   }

   return showmsg;
}

std::string canc_load_gen(IN xi::cli::context_t ctx, IN xi::Tl1Command &tl1cmd)
{
   if (0)
      DLOG("Purpose of removing waring: ctx[" << ctx << "] command[" << tl1cmd.Command() << "]");

   std::string showmsg;

   if (ih::LoadGenerator::Instance()->Command(td::loadcmd::STOP))
      showmsg = "success.\n";
   else
      showmsg = "fail.\n";

   return showmsg;
}

std::string rtrv_load_gen(IN xi::cli::context_t ctx, IN xi::Tl1Command &tl1cmd)
{
   if (0)
      DLOG("Purpose of removing waring: ctx[" << ctx << "] command[" << tl1cmd.Command() << "]");

   std::string showmsg;

   showmsg += "  * PLAN    : ";
   showmsg += ih::Property::Instance()->ToStringLoadScenario();
   showmsg += "\n";
   showmsg += "  * STATUS  : [1;33m";
   showmsg += ih::LoadGenerator::Instance()->GetStatus();
   showmsg += "[0m\n";
   showmsg += "\n";

   return showmsg;
}

std::string rtrv_debug(IN xi::cli::context_t ctx, IN xi::Tl1Command &tl1cmd)
{
   if (0)
      DLOG("Purpose of removing waring: ctx[" << ctx << "] command[" << tl1cmd.Command() << "]");

   std::string buffer;

   if (xi::StrEqualNoCase("HTTP1-STATUS", tl1cmd.Staging(0)))
   {
      buffer += InterfaceHub::Instance()->RetrieveDebug(tl1cmd.Staging(0));
      buffer += "\n";
   }
   else if (xi::StrEqualNoCase("HTTP1-DEBUG-STATISTICS", tl1cmd.Staging(0)))
   {
      buffer += InterfaceHub::Instance()->RetrieveDebug(tl1cmd.Staging(0));
      buffer += "\n";
   }
   else
   {
      //buffer += InterfaceHub::Instance()->RetrieveDebug("HTTP1-STATUS");
      //buffer += "\n";
      buffer += InterfaceHub::Instance()->RetrieveDebug("HTTP1-DEBUG-STATISTICS");
      buffer += "\n";
   }

   return buffer;
}


}
