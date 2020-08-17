// vim:ts=3:sts=3:sw=3

#ifndef TEMPLATE_HSM_HTTP_DETAIL_RECORDS_H_
#define TEMPLATE_HSM_HTTP_DETAIL_RECORDS_H_

#include <list>
#include <string>
#include <vector>
#include <unordered_map>
#include <tbb/atomic.h>
#include "xi/string.hxx"
#include "xi/rwlock.hxx"
#include "xi/singleton.hxx"

namespace hs {


class HttpDetailRecords : public xi::Singleton<HttpDetailRecords> {
   public :
      HttpDetailRecords();
      ~HttpDetailRecords();

      bool Load(const char *name, const char *file);
      std::string Summary(const uint32_t left_margin) const;
      bool Next(OUT xi::String &scheme, OUT xi::String &httpreq, OUT xi::String &content);
      bool Get(IN const char *loadkey, OUT xi::String &scheme, OUT xi::String &httpreq, OUT xi::String &content);

   private :
      bool LoadScenario(const char *file);
      bool LoadIngection(const char *file);
      uint32_t NextSeq();
      void Compose(const uint32_t rowid, OUT xi::String &scheme, OUT xi::String &httpreq, OUT xi::String &content) const;

   private :
      xi::RwLock lock_;

      xi::String name_;

      xi::String http_scheme_;
      xi::String http_request_;
      xi::String http_content_;

      char inject_delimiter_;
      xi::String inject_file_;
      xi::String key_column_;
      std::vector<std::string> header_;
      std::vector<std::string> value_rows_;
      uint32_t value_size_;
      std::unordered_map<std::string, uint32_t> key2idx_;

      // loadgen
      tbb::atomic<uint32_t> load_seq_;
};


}

#endif
