// vim:ts=3:sts=3:sw=3

#ifndef _MAIN_PROPERTY_H_
#define _MAIN_PROPERTY_H_ 

#include <list>
#include <map>
#include <set>
#include <string>
#include "rapidjson/document.h"
#include "xi/random.hxx"
#include "xi/rwlock.hxx"
#include "xi/singleton.hxx"
#include "xi/string.hxx"
#include "xi/socket_addr.hxx"
#include "rp/define.hxx"

#include "template/main/define.h"

namespace ih {


class Property : public xi::Singleton<Property>
{
   public :
      Property();
      ~Property();

      const char             *Version();

      // loading
      bool                    Load(char *config_file);

      // [system]
      xi::rp::pbgid_t         GetTopologyId();
      const char             *GetHostname();
      const char             *GetCliID();
      const char             *GetCliPW();
      uint16_t                GetCliPort();
      xi::loglevel::e         GetLogLevel();
      void                    SetLogLevel(xi::loglevel::e level);
      unsigned int            GetLogFileSize();
      void                    SetLogFileSize(unsigned int filesize);
      unsigned int            GetLogFileLimit();
      void                    SetLogFileLimit(unsigned int nLimit);

      // [resource]
      unsigned int            GetRpPoolSize();

      // [interface]
      uint16_t                GetHttpBindTcpPort();
      uint16_t                GetHttpBindTlsPort();
      const char             *GetHttpServerCertificate();
      const char             *GetHttpServerCertKey();
      void                    SetHttpServerCertificate(const char *file);
      void                    SetHttpServerCertKey(const char *file);

      // [timer]
      uint32_t                GetTimerValue(td::timer::e type);
      bool                    SetTimerValue(td::timer::e type, uint32_t value);

      // [service]
      std::list<test::Load>   GetLoadScenario();
      void                    SetLoadScenario(std::list<test::Load> &list);
      std::list<test::Load>   ParseLoadScenario(xi::String &cmd_line);
      std::string             ToStringLoadScenario();
      const char             *GetApi1Url() const;

      void                    ResetTestFlag();
      bool                    GetTestFlag(test::flag::toggle::e name) const;
      void                    SetTestFlag(test::flag::toggle::e name, bool value);
      int                     GetTestFlag(test::flag::number::e name) const;
      void                    SetTestFlag(test::flag::number::e name, int value);
      const char             *GetTestFlag(test::flag::letter::e name) const;
      void                    SetTestFlag(test::flag::letter::e name, const char *value);
      bool                    SameAsDefault(test::flag::toggle::e name);
      bool                    SameAsDefault(test::flag::number::e name);
      bool                    SameAsDefault(test::flag::letter::e name);

      bool                    NextAddr(xi::SocketAddr *out);
      void                    AddAddr(const xi::SocketAddr &addr);
      void                    RemoveAddr(const xi::SocketAddr &addr);
      void                    SetAddrLimit(uint32_t size);

   private :
      bool                    Load_system(IN rapidjson::Document &doc);
      bool                    Load_resource(IN rapidjson::Document &doc);
      bool                    Load_interface(IN rapidjson::Document &doc);
      bool                    Load_timer(IN rapidjson::Document &doc);
      bool                    Load_service(IN rapidjson::Document &doc);

      void                    Print();

   private :
      std::string             hostname_;
      std::string             config_file_;

      // system Section --------------------------------------------------------
      uint16_t                cli_port_;
      std::string             cli_id_;
      std::string             cli_pw_;

      // resource section ------------------------------------------------------
      unsigned int            rp_pool_size_;

      // interface section -----------------------------------------------------
      uint16_t                http_bind_tcp_port_;
      uint16_t                http_bind_tls_port_;
      xi::String              http_server_certificate_;
      xi::String              http_server_cert_key_;

      // timer section ---------------------------------------------------------
      xi::Random              random_;
      uint32_t                timer_value_[td::timer::END_OF_ENUM];

      // service section -------------------------------------------------------
      xi::String              test_api1_url_;

      // test section ----------------------------------------------------------
      std::list<test::Load>   load_scenario_list_;
      bool                    test_flag_toggle_[test::flag::toggle::END_OF_ENUM];
      int                     test_flag_number_[test::flag::number::END_OF_ENUM];
      xi::String              test_flag_letter_[test::flag::letter::END_OF_ENUM];

      // test
      xi::RwLock              conn_lock_;
      uint32_t                conn_max_size_;
      std::list<xi::SocketAddr> conn_lists_;
      uint32_t                conn_size_;
      uint32_t                conn_seqno_;
};


}
#endif
