// vim:ts=3:sts=3:sw=3

#include "manager.h"
#include "xi/logger.hxx"

namespace hs {


Manager::Manager()
{
}

Manager::~Manager()
{
}

bool Manager::Load(const char *apiname, const char *file)
{
   static const char *FN = "[HSMGR::Load] ";

   xi::RwLock::ScopedLockWrite lock(lock_);

   HttpDetailRecords *hdr = new HttpDetailRecords();
   if (false == hdr->Load(apiname, file))
      return false;

   auto it = table_.find(apiname);
   if (it != table_.end()) {
      WLOG(FN << "reload api:" << apiname);
      delete it->second;
      it->second = hdr;
   } else {
      table_.insert(std::pair<std::string, HttpDetailRecords*>(apiname, hdr));
   }

   return true;
}

std::string Manager::Summary(const char *apiname, const uint32_t left_margin) const
{
   xi::RwLock::ScopedLockRead lock(lock_);

   auto it = table_.find(apiname);
   if (it != table_.end())
      return it->second->Summary(left_margin);

   std::string empty;
   return empty;
}

bool Manager::Get(IN const char *apiname, OUT xi::String &scheme, OUT xi::String &httpreq, OUT xi::String &content)
{
   static const char *FN = "[HSMGR::Get] ";

   xi::RwLock::ScopedLockRead lock(lock_);

   auto it = table_.find(apiname);
   if (it != table_.end())
      return it->second->Next(scheme, httpreq, content);

   WLOG(FN << "not-found api:" << apiname);

   return false;
}

bool Manager::Get(IN const char *apiname, IN const char *loadkey, OUT xi::String &scheme, OUT xi::String &httpreq, OUT xi::String &content)
{
   static const char *FN = "[HSMGR::Get] ";

   xi::RwLock::ScopedLockRead lock(lock_);

   auto it = table_.find(apiname);
   if (it != table_.end())
      return it->second->Get(loadkey, scheme, httpreq, content);

   WLOG(FN << "not-found api:" << apiname);

   return false;
}


}
