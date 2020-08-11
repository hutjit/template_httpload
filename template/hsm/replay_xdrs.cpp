#include "replay_xdrs.h"
#include <stdio.h>
#include <stdlib.h>
#include "xi/logger.hxx"
#include "xi/pretty_table.hxx"
#include "xi/splitter.hxx"
#include "xi/util.hxx"
#include "template/main/property.h"

namespace hs {


ReplayXdrs::ReplayXdrs()
{
   count_ = 0;
   load_seq_ = 0;
}

ReplayXdrs::~ReplayXdrs()
{
}

bool ReplayXdrs::Load()
{
   const char *FN = "[ReplayXdrs::Load] ";

   return true;
}

void ReplayXdrs::Clear()
{
   xdrs_.clear();
   count_ = 0;
   load_seq_ = 0;
}

void ReplayXdrs::Print() const
{
   const char *FN = "[ReplayXdrs::Print] ";

   DLOG(FN << "rows : " << count_);

   if (0 == count_)
      return;

   const uint8_t delimiter = 0x1e; // record separator
   xi::PrettyTable table;

   for (uint32_t row = 0; row < count_; ++row) {
      const std::string &xdr = xdrs_[row];

      xi::Splitter split(xdr.c_str(), (const char)delimiter);
      const uint32_t col_size = split.GetSize();


      // body 생성
      for (uint32_t col = 0; col < split.GetSize(); ++col) {
         // header 생성
         if (0 == row) {
            char buffer[32];
            snprintf(buffer, sizeof(buffer), "col-%02d", col + 1);
            table.SetHead(col, buffer);
         }

         std::string value;
         if (!xi::IsZero(split[col]))
            value = split[col];

         table.SetColumnValue(row, col, value);
      }
   }

   DLOG(FN << "\n" << table.Print());
}

const std::string *ReplayXdrs::At(IN uint32_t index) const
{
   if (0 == count_)
      return NULL;

   return &xdrs_[index % count_];
}

const std::string *ReplayXdrs::Next()
{
   if (0 == count_)
      return NULL;

   uint32_t next_seq = (load_seq_++) % count_;
   return At(next_seq);
}


}
