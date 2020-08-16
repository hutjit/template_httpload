// vim:ts=3:sts=3:sw=3

#include "load_generator.h"
#include <stdio.h>
#include <typeinfo>
#include "xi/datetime.hxx"
#include "xi/logger.hxx"
#include "xi/util.hxx"
#include "template/ifm/load_command.h"
#include "interface_hub.h"
#include "property.h"

namespace ih {


LoadGenerator::LoadGenerator()
   : generator_(NULL)
{
}

LoadGenerator::~LoadGenerator()
{
   if (generator_) {
      delete generator_;
      generator_ = NULL;
   }
}

bool LoadGenerator::Initialize()
{
   static const char *FN = "[LoadGen::Initialize] ";

   if (generator_) {
      WLOG(FN << "already up !!");
      return false;
   }

   generator_ = xi::CreateThread(128*1024, this, &LoadGenerator::Generator);
   if (NULL == generator_) {
      WLOG(FN << "CreateThread(Generator) fail");
      return false;
   }

   return true;
}

bool LoadGenerator::Command(td::loadcmd::e loadcmd, test::scenario::e scenario, unsigned cps, std::string option)
{
   static const char *FN = "[LoadGen::Command] ";

   Control cmd;
   cmd.loadcmd_ = loadcmd;
   cmd.cps_ = cps;
   cmd.option_ = option;

   switch (loadcmd) {
      case td::loadcmd::START :
         {
            WOUT(FN << td::loadcmd::name(loadcmd) << " cps:" << cps << " Option:" << option);

            {
               xi::Mutex::ScopedLock lock(load_test_lock_);
               load_scenario_list_ = ih::Property::Instance()->GetLoadScenario();
            }

            xi::Mutex::ScopedLock lock(load_property_.lock_);
            load_property_.next_control_commands_.push_back(cmd);
         } break;

      case td::loadcmd::STOP :
         {
            WOUT(FN << td::loadcmd::name(loadcmd));

            xi::Mutex::ScopedLock lock(load_property_.lock_);
            load_property_.next_control_commands_.push_back(cmd);
         } break;

      case td::loadcmd::UNIT_TEST :
         {
            ifm::LoadCommand *load_cmd = new ifm::LoadCommand;
            load_cmd->SetScenario(scenario);

            WOUT(FN << td::loadcmd::name(loadcmd) << " " << test::scenario::name(scenario) << " Option:" << option);
            if (false == option.empty())
            {
               load_cmd->SetUser(option.c_str());
               // if (NULL == hs::Manager::Instance()->Find(option.c_str(), option.size())) {
               //    WLOG(FN << "not-found eui:" << option);
               //    delete load_cmd;
               //    return false;
               // }
            }

            std::unique_ptr<xi::rp::Payload> primitive(load_cmd);
            InterfaceHub::Instance()->ExecThread(std::move(primitive));

         } break;

      default :
         {
            WLOG(FN << "Unknown CMD:" << (int)loadcmd);
            return false;
         } break;
   }

   return true;
}

void LoadGenerator::CheckCommand(IN Property &property, OUT td::loadcmd::e &loadcmd, OUT uint32_t &cps)
{
   static const char *FN = "[LoadGen::CheckCommand] ";

   xi::Mutex::ScopedLock lock(property.lock_);
   if (property.next_control_commands_.empty()) {
      loadcmd = property.current_loadcmd_;
      cps = property.cps_;
      return;
   }

   Control ctrl = property.next_control_commands_.front();
   property.next_control_commands_.pop_front();

   loadcmd = ctrl.loadcmd_;
   cps = ctrl.cps_;
   property.current_loadcmd_ = loadcmd;
   WOUT(FN << td::loadcmd::name(loadcmd) << ", cps:" << cps << ", OPTION:" << ctrl.option_);

   switch (loadcmd) {
      case td::loadcmd::START :
         {
            property.start_time_ = time(NULL);
            property.stop_time_ = 0;
            property.cps_ = cps;
            property.trun_count_ = (ctrl.option_.empty() ? 0 : atoi(ctrl.option_.c_str()));
         } break;

      case td::loadcmd::STOP :
         {
            property.stop_time_ = time(NULL);
         } break;

      default :
         {
            WLOG(FN << "Unsupported yet");
         } break;
   }
}

void LoadGenerator::Generator()
{
   static const char *FN = "[LoadGen::Generator] ";

   td::loadcmd::e loadcmd = td::loadcmd::NONE;
   uint32_t cps = 0;
   time_t prev_time = time(NULL);
   time_t prev_min = 0;

   do {
      time_t now = time(NULL);
      if (prev_time == now) {
         xi::Sleep(10);
         continue;
      }
      prev_time = now;

      CheckCommand(load_property_, loadcmd, cps);

      // 매분마다 1번씩만 실행
      if (ih::Property::Instance()->GetTestFlag(test::flag::toggle::TR_PER_MINUTE)) {
         time_t now_min = now - (now % 60);
         if (prev_min == now_min)
            continue;

         prev_min = now_min;
      }

      switch (loadcmd) {
         case td::loadcmd::STOP :
            {
               continue;
            } break;

         case td::loadcmd::START :
            {
               xi::Mutex::ScopedLock lock(load_test_lock_);

               if (load_scenario_list_.empty())
               {
                  WLOG(FN << "load_scenario_list_ is empty");

                  xi::Mutex::ScopedLock lock(load_property_.lock_);
                  Control ctrl;
                  ctrl.loadcmd_ = td::loadcmd::STOP;
                  load_property_.next_control_commands_.push_back(ctrl);
                  continue;
               }

               std::list<test::Load>::iterator iter = load_scenario_list_.begin();
               test::Load load_scenario = *iter;

               for (; cps; --cps)
               {
                  if (0 == load_scenario.turn_)
                  {
                     ++iter;
                     if (iter == load_scenario_list_.end())
                        iter = load_scenario_list_.begin();
                     load_scenario = *iter;
                  }
                  else
                  {
                     --load_scenario.turn_;
                  }

                  DLOG(FN << "load remain cps:" << cps << " turn:" << load_scenario.turn_);

                  // Command-Trigger 
                  ifm::LoadCommand *load_cmd = new ifm::LoadCommand;
                  load_cmd->SetScenario(load_scenario.scenario_);

                  std::unique_ptr<xi::rp::Payload> primitive(load_cmd);
                  InterfaceHub::Instance()->ExecThread(std::move(primitive));
               }

               if (load_property_.trun_count_) {
                  if (0 == --load_property_.trun_count_) {
                     DLOG(FN << "Turn END");
                     xi::Mutex::ScopedLock lock(load_property_.lock_);
                     Control ctrl;
                     ctrl.loadcmd_ = td::loadcmd::STOP;
                     load_property_.next_control_commands_.push_back(ctrl);
                  }
               }

            } break;

         default :
            {
               WLOG(FN << "Unknown CMD:" << loadcmd << " cps:" << cps);
            } break;
      }
   } while (1);
}

std::string LoadGenerator::GetStatus()
{
   xi::String output;
   time_t now = time(NULL);

   xi::Mutex::ScopedLock lock(load_property_.lock_);

   if (load_property_.start_time_ && (load_property_.start_time_ > load_property_.stop_time_))
   {
      xi::DateTime dt(load_property_.start_time_);

      char convtime[32];
      time_t duration = now - load_property_.start_time_;

      snprintf(convtime, sizeof(convtime), "%02d:%02d:%02d", (int)duration / 3600, (int)(duration % 3600) / 60, (int)duration % 60);
      output.Csnprintf(512, "%s load(started), %d cps, %s", dt.ToString("%Y/%m/%d %H:%M:%S"), load_property_.cps_, convtime);

   }
   else if (load_property_.stop_time_ && (load_property_.start_time_ <= load_property_.stop_time_))
   {
      xi::DateTime dt(load_property_.stop_time_);

      if (load_property_.start_time_)
      {
         char convtime[32];
         time_t duration = load_property_.stop_time_ - load_property_.start_time_;
         sprintf(convtime, "%02d:%02d:%02d", (int)duration / 3600, (int)(duration % 3600) / 60, (int)duration % 60);

         output.Csnprintf(512, "%s load(stopped), %d cps, %s", dt.ToString("%Y/%m/%d %H:%M:%S"), load_property_.cps_, convtime);
      }
      else
      {
         output.Csnprintf(512, "%s load(stopped), %d cps", dt.ToString("%Y/%m/%d %H:%M:%S"), load_property_.cps_);
      }
   }
   else
   {
      output = "load(idle)";
   }

   return output.c_log();
}


}
