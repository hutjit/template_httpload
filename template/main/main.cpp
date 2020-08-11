// vim:ts=3:sts=3:sw=3

#include <stdio.h>
#include <signal.h>
#include <errno.h>
#include <iostream>
#include "xi/logger.hxx"
#include "xi/util.hxx"
#include "property.h"
#include "cli.h"
#include "interface_hub.h"
#include "rp_pool.h"
#include "statistics.h"
#include "revision.h"
#include "template/hsm/replay_xdrs.h"


extern char *program_invocation_short_name;
char g_pidfile_path[256] = {0, };

// ---------------------------------------------------------------------------
// signal handler 
// ---------------------------------------------------------------------------
struct sigaction sigact_new[256];
struct sigaction sigact_old[256];

void sig_core_handler(int signo)
{
   switch (signo) {
      case 9  :   // SIGKILL
      case 15 :   // SIGTERM
         {
            EOUT("**** SIGNAL(" << signo << ":" << strsignal(signo) << ") shutdown ****");
            xi::Sleep(200);
            if (false == xi::IsZero(g_pidfile_path))
               xi::DeletePidFile(g_pidfile_path);
            exit(0);
         } break;

      default :
         {
            (sigact_old[signo].sa_handler)(signo);
         } break;
   }
}

#define SIGACT_SET(signo) { \
   sigact_new[signo].sa_handler = sig_core_handler; \
   sigemptyset(&sigact_new[signo].sa_mask); \
   sigaction(signo, &(sigact_new[signo]), &(sigact_old[signo])); \
}
//  1) SIGACT_SET(SIGHUP);
//  2) SIGACT_SET(SIGINT);
//  3) SIGACT_SET(SIGQUIT);
//  6) SIGACT_SET(SIGABRT);
//  9) SIGACT_SET(SIGKILL);
// 11) SIGACT_SET(SIGSEGV);
// 15) SIGACT_SET(SIGTERM);
// ---------------------------------------------------------------------------

void exit_handler(void)
{
   // log flush
   xi::Sleep(200);
}

void Usage(char *procname)
{
   const char *build_tag =
      "\n"
      "    BUILD-TAG\n"
      "    * version[" PACKAGE_STRING "]\n"
      "    * compile-date[" __DATE__ " " __TIME__ "]\n"
      "    * revision[" REPOSITORY_REVISION "]\n"
      ;

   std::cout << "\n---------------------------------------------------" << std::endl;
   std::cout << "Usage: " << procname << " -c [config-filename] -l [log-path]" << std::endl;
   std::cout << procname << " -c config.template.cfg -l ./" << std::endl;
   std::cout << "---------------------------------------------------" << std::endl;
   std::cout << build_tag << std::endl;
}

int main(int argc, char **argv)
{
   std::cout << std::endl;
   memset(g_pidfile_path, 0x00, sizeof(g_pidfile_path));

   char *config_file = NULL;
   char *log_path = NULL;

   // ---------------------------------------------------------------------------
   // check Options
   char c = 0x00;
   while ((c = getopt(argc, argv, "s:c:l:")) != EOF)
   {
      switch(c)
      {
         case 'c': config_file = optarg; break;
         case 'l': log_path    = optarg; break;
         case 's':                       break;
         default : { Usage(argv[0]); _EXIT(1); }
      }
   }

   if (NULL == config_file) {
      std::cout << program_invocation_short_name << " start fail. not-found [config-filename]" << std::endl;
      Usage(argv[0]);
      _EXIT(1);
   }

   if (NULL == log_path) {
      std::cout << program_invocation_short_name << " start fail. not-found [log-path]" << std::endl;
      Usage(argv[0]);
      _EXIT(1);
   }

   // ---------------------------------------------------------------------------
   // set Log
   {
      char LOGPATH[256];

      snprintf(LOGPATH, sizeof(LOGPATH), "%s", log_path);
      int len = strlen(LOGPATH);
      if ('/' == LOGPATH[len-1]) {
         LOGPATH[len-1] = 0x00;
         len--;
      }

      strncpy(g_pidfile_path, LOGPATH, len);

      snprintf(&LOGPATH[len], sizeof(LOGPATH) - len, "/debug.%s.log", program_invocation_short_name);
      snprintf(&g_pidfile_path[len], sizeof(g_pidfile_path), "/%s.pid", program_invocation_short_name);

      SETLOGPATH(LOGPATH);
      SETLOGLEVEL(xi::loglevel::DEBUG);
   }

   {
      xi::String created = __DATE__ " " __TIME__;
      xi::String version_print;
      version_print.Csnprintf(2048,
            "\n\n"
            "#=====================================================#\n"
            "#  XDRGEN                                             #\n"
            "#-----------------------------------------------------#\n"
            "#  - version    : %-35s #\n"
            "#  - created    : %-35s #\n"
            "#  - repository : %-35s #\n"
            "#=====================================================#\n"
            "\n"
            "  - %s\n"
            "\n"
            , PACKAGE_VERSION
            , created.c_log()
            , REPOSITORY_REVISION
            , xi::Version()
            );

      EOUT(version_print);
   }

   // ===========================================================================
   // 0. load Config
   // ---------------------------------------------------------------------------
   ih::Property *property = ih::Property::Instance();
   if (!property->Load(config_file)) {
      WLOG("Read Config[" << config_file << "] fail");
      _EXIT(1);
   }

   // ===========================================================================
   // 1. Pool initialize
   // ---------------------------------------------------------------------------
   ih::RpPool *rp_pool = ih::RpPool::Instance();
   if (!rp_pool->Initialize(property->GetRpPoolSize())) {
      WLOG("IhPool::Initialize() fail");
      _EXIT(1);
   }
   rp_pool->SetHaActive();

   if (NULL == ih::Statistics::Instance()) {
      WLOG("Statistics::Initialize() fail");
      _EXIT(1);
   }


   // ===========================================================================
   // 2. CLI initialize
   // ---------------------------------------------------------------------------
   if (false == ih::Cli::Instance()->Initialize()) {
      WLOG("CLI::Initialize() fail");
      _EXIT(1);
   }


   // ===========================================================================
   // 3. InterfaceHub initialize
   // ---------------------------------------------------------------------------
   if (false == ih::InterfaceHub::Instance()->Initialize()) {
      WLOG("InterfaceHub::Initialize() fail");
      _EXIT(1);
   }


   // ===========================================================================
   // 4. xdr scenario load
   // ---------------------------------------------------------------------------
 
   if (false == hs::ReplayXdrs::Instance()->Load()) {
      WLOG("ReplayXdrs::Load() fail");
      _EXIT(1);
   }
   //hs::ReplayXdrs::Instance()->Print();


   xi::CreatePidFile(g_pidfile_path);

   EOUT   (" ");
   EOUT   ("---------- " << program_invocation_short_name << " : SERVICE-READY ----------");
   EOUT   (" ");

   // ---------------------------------------------------------------------------
   // register Signal Handler
   signal(SIGPIPE, SIG_IGN);

   SIGACT_SET(SIGHUP);
   SIGACT_SET(SIGQUIT);
   SIGACT_SET(SIGABRT);
   SIGACT_SET(SIGKILL);
   SIGACT_SET(SIGSEGV);
   SIGACT_SET(SIGTERM);


   // ---------------------------------------------------------------------------
   // house keeping
   WOUT("\n" << ih::RpPool::Instance()->Status());
   while (1)
   {
      sleep(60);
      DLOG("\n" << ih::RpPool::Instance()->Status());
   }

   return 0;
}
