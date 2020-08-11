#ifndef _IFM_KAFKA_PRODUCE_H_
#define _IFM_KAFKA_PRODUCE_H_

#include <string>
#include <vector>
#include "xi/bytes.hxx"
#include "rp/payload.hxx"
#include "template/ifm/define.h"

namespace ifm {


class KafkaProduce : public xi::rp::Payload
{
   public :
      KafkaProduce();
      virtual ~KafkaProduce();

      virtual xi::rp::Payload *Clone();

      uint32_t                Size() const;
      void                    Push(xi::Bytes *message);
      const xi::Bytes        *GetMessage(uint32_t index) const;

   private :
      std::vector<xi::Bytes*> messages_;
      uint32_t                size_;
};


}

#endif
