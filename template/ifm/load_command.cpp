// vim:ts=3:sts=3:sw=3

#include "load_command.h"
#include "xi/util.hxx"

namespace ifm {


LoadCommand::LoadCommand()
   : xi::rp::Payload(ifm::msgtype::LOAD_COMMAND)
{
   scenario_ = 0;
}

LoadCommand::~LoadCommand()
{
   user_.clear();
   recipient_list_.clear();
}

xi::rp::Payload *LoadCommand::Clone()
{
   LoadCommand *pClone = new LoadCommand();
   *pClone = *this;

   return pClone;
}

int LoadCommand::GetScenario()
{
   return scenario_;
}

const char *LoadCommand::GetUser()
{
   return (user_.empty() ? NULL : user_.c_str());
}

const char *LoadCommand::GetUserParam()
{
   return (user_param_.empty() ? NULL : user_param_.c_str());
}

void LoadCommand::GetRecipientList(std::set<std::string> &list)
{
   list = recipient_list_;
}

void LoadCommand::SetScenario(int scenario)
{
   scenario_ = scenario;
}

void LoadCommand::SetUser(const char *uri)
{
   if (!xi::IsZero(uri))
      user_ = uri;
   else
      user_.clear();
}

void LoadCommand::SetUserParam(const char *param)
{
   if (false == xi::IsZero(param))
      user_param_ = param;
   else
      user_param_.clear();
}

void LoadCommand::AddRecipient(const char *uri)
{
   if (!xi::IsZero(uri))
      recipient_list_.insert(uri);
}


}
