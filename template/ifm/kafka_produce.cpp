// vim:ts=3:sts=3:sw=3

#include "kafka_produce.h"

namespace ifm {


KafkaProduce::KafkaProduce() : xi::rp::Payload(ifm::msgtype::KAFKA_PRODUCE)
{
   size_ = 0;
}

KafkaProduce::~KafkaProduce()
{
   for (auto &v : messages_) {
      delete v;
   }
}

xi::rp::Payload *KafkaProduce::Clone()
{
   KafkaProduce *clone = new KafkaProduce();

   *clone = *this;

   return clone;
}


uint32_t KafkaProduce::Size() const
{
   return size_;
}

void KafkaProduce::Push(xi::Bytes *message)
{
   messages_.push_back(message);
   size_ = messages_.size();
}

const xi::Bytes *KafkaProduce::GetMessage(uint32_t index) const
{
   if (size_ <= index)
      return NULL;

   return messages_[index];
}


}
