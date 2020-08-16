// vim:ts=3:sts=3:sw=3

#ifndef TEMPLATE_IFM_LOAD_COMMAND_H_
#define TEMPLATE_IFM_LOAD_COMMAND_H_

#include <string>
#include <set>
#include "rp/payload.hxx"
#include "template/ifm/define.h"

namespace ifm {


class LoadCommand : public xi::rp::Payload
{
   public :
      LoadCommand();
      virtual ~LoadCommand();

      virtual xi::rp::Payload  *Clone();

      int                   GetScenario();
      const char           *GetUser();
      const char           *GetUserParam();
      void                  GetRecipientList(std::set<std::string> &list);

      void                  SetScenario(int scenario);
      void                  SetUser(const char *uri);
      void                  SetUserParam(const char *param);
      void                  AddRecipient(const char *uri);

   private :              
      int                   scenario_;
      std::string           user_;
      std::string           user_param_;
      std::set<std::string> recipient_list_;
};


}

#endif
