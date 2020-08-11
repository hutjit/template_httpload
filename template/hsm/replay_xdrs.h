#ifndef _HSM_REPLAY_XDRS_H_
#define _HSM_REPLAY_XDRS_H_

#include <vector>
#include <list>
#include <string>
#include <tbb/atomic.h>
#include "xi/singleton.hxx"

namespace hs {


class ReplayXdrs : public xi::Singleton<ReplayXdrs> {
   public :
      ReplayXdrs();
      ~ReplayXdrs();

      bool Load();
      void Clear();
      void Print() const;
      const std::string *At(IN uint32_t index) const;
      const std::string *Next();

   private :
      std::vector<std::string> xdrs_;
      uint32_t count_;
      tbb::atomic<uint32_t> load_seq_;
};


}

#endif
