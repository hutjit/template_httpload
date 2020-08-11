// vim:ts=3:sts=3:sw=3

#ifndef _TUM_MANAGER_H_
#define _TUM_MANAGER_H_

#include "xi/singleton.hxx"
#include "rp/payload.hxx"

namespace tu {


class Manager : public xi::Singleton<Manager>
{
   public :
      Manager();
      ~Manager();

      xi::rp::result::e OnReceive(IN xi::rp::Payload &message, OUT xi::rp::membid_t &mid);

   private :
};


}

#endif
