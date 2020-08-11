// vim:ts=3:sts=3:sw=3

#include "manager.h"
#include "xi/logger.hxx"
#include "template/tum/session.h"
#include "template/main/session.h"
#include "template/main/rp_pool.h"
#include "template/main/interface_hub.h"

namespace tu {


Manager::Manager()
{
}

Manager::~Manager()
{
}

xi::rp::result::e Manager::OnReceive(IN xi::rp::Payload &message, OUT xi::rp::membid_t &mid)
{
   static const char *FN = "[TUMGR::OnReceive] ";
   static ih::RpPool *rp_pool = ih::RpPool::Instance();

   ih::Session *container = static_cast<ih::Session*>(message.GetSessReference());
   if (NULL == container) {
      WLOG(FN << "ih::Session(null)");
      return xi::rp::result::INVALID_ARGS;
   }

   tu::Session *tusess = static_cast<tu::Session*>(rp_pool->Obtain(td::functype::TU_SESSION));
   if (NULL == tusess) {
      WLOG(FN << "tu::Session Obtain fail");
      return xi::rp::result::RESOURCE_OBTAIN_FAIL;
   }

   mid = container->Attach(tusess);
   if (0 == mid) {
      WLOG(FN << "Attach FAIL");
      return xi::rp::result::RESOURCE_ATTACH_FAIL;
   }

   return tusess->OnReceive(message);
}


}
