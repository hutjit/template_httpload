// vim:ts=3:sts=3:sw=3

#ifndef TEMPLATE_MAIN_RESOURCE_POOL_H_
#define TEMPLATE_MAIN_RESOURCE_POOL_H_

#include "xi/singleton.hxx"
#include "rp/pool.hxx"

namespace ih {


class RpPool : public xi::Singleton<RpPool>
{
   public :
      RpPool();
      ~RpPool();
      bool                    Initialize(unsigned int nPoolSize);
      void                    SetHaActive();
      bool                    IsHaActive();

      xi::rp::Element        *Obtain(xi::rp::resource_t type);
      void                    Release(xi::rp::Element *pObj);
      xi::rp::Element        *Find(xi::rp::resource_t type, xi::rp::sessid_t id);

      std::string             Status();

   private :
      xi::rp::Pool           *impl_;
};


}
#endif
