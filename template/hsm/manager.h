// vim:ts=3:sts=3:sw=3

#ifndef TEMPLATE_HSM_MANAGER_H_
#define TEMPLATE_HSM_MANAGER_H_

#include <map>
#include <string>
#include "xi/rwlock.hxx"
#include "xi/singleton.hxx"
#include "rp/payload.hxx"
#include "template/hsm/http_detail_records.h"

namespace hs {


class Manager : public xi::Singleton<Manager>
{
   public :
      Manager();
      ~Manager();

      bool Load(const char *apiname, const char *file);
      std::string Summary(const char *apiname, const uint32_t left_margin) const;
      bool Next(IN const char *apiname,  OUT xi::String &scheme, OUT xi::String &httpreq, OUT xi::String &content);

   private :
      xi::RwLock lock_;
      std::map<std::string, HttpDetailRecords*> table_;
};


}

#endif
