// vim:ts=3:sts=3:sw=3

#ifndef TEMPLATE_MAIN_LOAD_GENERATOR_H_
#define TEMPLATE_MAIN_LOAD_GENERATOR_H_

#include <stdint.h>
#include <list>
#include <string>
#include "xi/mutex.hxx"
#include "xi/singleton.hxx"
#include "xi/string.hxx"
#include "xi/thread.hxx"
//#include "rp/payload.hxx"
#include "template/main/define.h"

namespace ih {


class LoadGenerator : public xi::Singleton<LoadGenerator>
{
   public :
      class Control {
         public :
            td::loadcmd::e        loadcmd_;
            unsigned int          cps_;
            std::string           option_;

            Control() : loadcmd_(td::loadcmd::NONE), cps_(0) {}
      };

      class Property {
         public :
            xi::Mutex             lock_;
            std::list<Control>    next_control_commands_;
            td::loadcmd::e        current_loadcmd_;
            unsigned int          cps_;
            unsigned int          trun_count_;

            time_t                start_time_;
            time_t                stop_time_;

            Property() : current_loadcmd_(td::loadcmd::STOP), cps_(0), trun_count_(0), start_time_(0), stop_time_(0) {}
      };

   public :
      LoadGenerator();
      ~LoadGenerator();

      bool                    Initialize();
      void                    Generator();

      std::string             GetStatus();
      bool                    Command(td::loadcmd::e loadcmd, test::scenario::e scenario = test::scenario::NONE, unsigned cps = 0, std::string option = "");

   private :
      void                    CheckCommand(IN Property &property, OUT td::loadcmd::e &loadcmd, OUT uint32_t &cps);

   private :
      xi::Thread             *generator_;

      xi::Mutex               load_test_lock_;
      Property                load_property_;
      std::list<test::Load>   load_scenario_list_;
};


}
#endif
