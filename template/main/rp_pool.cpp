// vim:ts=3:sts=3:sw=3

#include "rp_pool.h"
#include "xi/logger.hxx"
#include "xi/pretty_table.hxx"
#include "template/tum/session.h"
#include "template/main/session.h"

namespace ih {


RpPool::RpPool()
{
   impl_ = xi::rp::Pool::Instance();
}

RpPool::~RpPool()
{
   impl_ = NULL;
}

xi::rp::Element *RpPool::Obtain(xi::rp::resource_t type)
{
   return impl_->Obtain(type);
}

void RpPool::Release(xi::rp::Element *pObj)
{
   impl_->Release(pObj);
}

xi::rp::Element *RpPool::Find(xi::rp::resource_t type, xi::rp::sessid_t id)
{
   return impl_->Find(type, id);
}

bool RpPool::Initialize(unsigned int nPoolSize)
{
   for (unsigned int i = 0; i < nPoolSize; ++i) {
      {
         ih::Session *element = new ih::Session;
         impl_->Push(element);
      }

      {
         tu::Session *element = new tu::Session;
         impl_->Push(element);
      }
   }

   return true;
}

void RpPool::SetHaActive()
{
   if (impl_)
      impl_->SetHaActive();
}     

bool RpPool::IsHaActive()
{
   return (impl_ ? impl_->IsHaActive() : false);
}

std::string RpPool::Status()
{
   xi::PrettyTable table;
   table.SetMarginLeft(2);
   table.SetHead(0, "TYPE");
   table.SetHead(1, "BUSY");
   table.SetHead(2, "IDLE");
   table.SetHead(3, "TOTAL");
   table.SetWidth(1, 8);
   table.SetWidth(2, 8);
   table.SetWidth(3, 8);
   table.SetAlign(1, xi::PrettyTable::kRight);
   table.SetAlign(2, xi::PrettyTable::kRight);
   table.SetAlign(3, xi::PrettyTable::kRight);

   uint32_t row_index = 0;
   uint32_t total, idle, busy;

   {
      impl_->GetStatus(td::functype::IH_SESSION, total, idle, busy);
      table.SetColumnValue(row_index, 0, "IH-SESSION");
      table.SetColumnValue(row_index, 1, (uint64_t) busy);
      table.SetColumnValue(row_index, 2, (uint64_t) idle);
      table.SetColumnValue(row_index, 3, (uint64_t) total);
      row_index++;
   }
   {
      impl_->GetStatus(td::functype::TU_SESSION, total, idle, busy);
      table.SetColumnValue(row_index, 0, "TU-SESSION");
      table.SetColumnValue(row_index, 1, (uint64_t) busy);
      table.SetColumnValue(row_index, 2, (uint64_t) idle);
      table.SetColumnValue(row_index, 3, (uint64_t) total);
      row_index++;
   }

   return table.Print();
}


}
