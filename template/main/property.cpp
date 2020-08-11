// vim:ts=3:sts=3:sw=3

#include "property.h"
#include <stdio.h>
#include <unistd.h>
#include <fstream>
#include "rapidjson/writer.h"
#include "rapidjson/stringbuffer.h"
#include "rapidjson/istreamwrapper.h"
#include "xi/datetime.hxx"
#include "xi/logger.hxx"
#include "xi/util.hxx"
#include "xi/splitter.hxx"
#include "xi/socket_addr.hxx"
#include "xi/input_string_cursor.hxx"
#include "revision.h"
#include "interface_hub.h"

namespace ih {


Property::Property()
{
   hostname_ = "(unknown)";
   rp_pool_size_ = 1000;

   // ===========================================================================
   // Default Timer value
   // ---------------------------------------------------------------------------
   for (int i = td::timer::NONE + 1; i < td::timer::END_OF_ENUM; ++i) {
      timer_value_[i] = td::timer::default_value((td::timer::e) i);
   }
   // ---------------------------------------------------------------------------

   ResetTestFlag();

   { // gethostname
      char hostName[128];
      memset(hostName, 0 ,sizeof(hostName));
      if (gethostname(hostName, sizeof(hostName)) == 0)
      {
         if (hostName[0])
            hostname_ = hostName;
      }
   }

   http_bind_tcp_port_ = 0;
   http_bind_tls_port_ = 0;

   conn_max_size_ = 100;
   conn_size_ = 0;
   conn_seqno_ = 0;
}

Property::~Property()
{
}

const char *Property::Version()
{
   return PACKAGE_STRING;
}

bool Property::Load(char *config_file)
{
   static const char *FN = "[property::Load] ";

   if (xi::IsZero(config_file)) {
      WLOG(FN << "invalid file(null)");
      return false;
   }

   config_file_  = config_file;

   // https://stackoverflow.com/questions/18107079/rapidjson-working-code-for-reading-document-from-file
   std::ifstream ifs(config_file_.c_str());
   rapidjson::IStreamWrapper isw(ifs);

   rapidjson::Document doc;

   if (doc.ParseStream(isw).HasParseError()) {
      WLOG(FN << "xi::Config::LoadFile(\"" << config_file_ << "\") fail");
      return false;
   }


   // ----- system -----
   if (!Load_system(doc)) {
      WLOG(FN << "Load_system() fail");
      return false;
   }

   // ----- resource -----
   if (!Load_resource(doc)) {
      WLOG(FN << "Load_resource() fail");
      return false;
   }

   // ----- interface -----
   if (!Load_interface(doc)) {
      WLOG(FN << "Load_interface() fail");
      return false;
   }

   // ----- timer -----
   if (!Load_timer(doc)) {
      WLOG(FN << "Load_timer() fail");
      return false;
   }

   // ----- service -----
   if (!Load_service(doc)) {
      WLOG(FN << "Load_service() fail");
      return false;
   }


   Print();

   return true;
}

bool Property::Load_system(rapidjson::Document &doc)
{
   static const char *FN = "[property::Load_system] ";

   std::string key;

   key = "system";
   rapidjson::Value::MemberIterator section = doc.FindMember(key.c_str());
   if (section == doc.MemberEnd()) {
      WLOG(FN << "not-found " << key);
      return false;
   }

   // 1. Log
   {
      key = "log.level";
      rapidjson::Value::MemberIterator found = section->value.FindMember(key.c_str());
      if ((found == section->value.MemberEnd()) || (false == found->value.IsString())) {
         WLOG(FN << "not-found " << key);
         return false;
      }
      SetLogLevel(xi::loglevel::code(found->value.GetString()));
   }
   {
      key = "log.max-file-archive";
      rapidjson::Value::MemberIterator found = section->value.FindMember(key.c_str());
      if ((found != section->value.MemberEnd()) && found->value.IsNumber()) {
         SetLogFileLimit((unsigned int)found->value.GetInt());
      } else {
         WLOG(FN << "not-found " << key);
      }
   }
   {
      key = "log.max-file-size";
      rapidjson::Value::MemberIterator found = section->value.FindMember(key.c_str());
      if ((found != section->value.MemberEnd()) && found->value.IsNumber()) {
         SetLogFileSize((unsigned int)found->value.GetInt());
      } else {
         WLOG(FN << "not-found " << key);
      }
   }

   // 2. CLI
   // 2.1. CLI.Port
   {
      key = "cli.bind-port";
      rapidjson::Value::MemberIterator found = section->value.FindMember(key.c_str());
      if ((found == section->value.MemberEnd()) || (false == found->value.IsNumber())) {
         WLOG(FN << "not-found " << key);
         return false;
      }
      cli_port_ = (uint16_t) found->value.GetInt();
   }

   // 2.2. CLI Auth ID
   {
      key = "cli.login-id";
      rapidjson::Value::MemberIterator found = section->value.FindMember(key.c_str());
      if ((found == section->value.MemberEnd()) || (false == found->value.IsString())) {
         WLOG(FN << "not-found " << key);
         return false;
      }
      cli_id_ = found->value.GetString();
   }

   // 2.3. CLI Auth PW
   {
      key = "cli.login-pw";
      rapidjson::Value::MemberIterator found = section->value.FindMember(key.c_str());
      if ((found == section->value.MemberEnd()) || (false == found->value.IsString())) {
         WLOG(FN << "not-found " << key);
         return false;
      }
      cli_pw_ = found->value.GetString();
   }

   return true;
}

bool Property::Load_resource(rapidjson::Document &doc)
{
   static const char *FN = "[property::Load_resource] ";

   std::string key;

   key = "resource";
   rapidjson::Value::MemberIterator section = doc.FindMember(key.c_str());
   if (section == doc.MemberEnd()) {
      WLOG(FN << "not-found " << key);
      return false;
   }


   {
      key = "rp.pool-size";
      rapidjson::Value::MemberIterator found = section->value.FindMember(key.c_str());
      if ((found == section->value.MemberEnd()) || (false == found->value.IsNumber())) {
         WLOG(FN << "not-found " << key);
         return false;
      }
      rp_pool_size_ = (unsigned) found->value.GetUint();
   }

   return true;
}

bool Property::Load_interface(rapidjson::Document &doc)
{
   static const char *FN = "[property::Load_interface] ";

   std::string key;

   key = "interface";
   rapidjson::Value::MemberIterator section = doc.FindMember(key.c_str());
   if (section == doc.MemberEnd()) {
      WLOG(FN << "not-found " << key);
      return false;
   }

   {
      key = "http.bind-tcp-port";
      rapidjson::Value::MemberIterator found = section->value.FindMember(key.c_str());
      if ((found != section->value.MemberEnd()) && found->value.IsNumber()) {
         http_bind_tcp_port_ = (uint16_t) found->value.GetUint();
      }
   }

   {
      key = "http.bind-tls-port";
      rapidjson::Value::MemberIterator found = section->value.FindMember(key.c_str());
      if ((found != section->value.MemberEnd()) && found->value.IsNumber()) {
         http_bind_tls_port_ = (uint16_t) found->value.GetUint();
      }
   }

   {
      key = "http-server-certificate";
      rapidjson::Value::MemberIterator found = section->value.FindMember(key.c_str());
      if ((found != section->value.MemberEnd()) && found->value.IsString()) {
         xi::String value = found->value.GetString();
         value.Trim();
         if (!value.IsEmpty()) {
            SetHttpServerCertificate(value.c_str());
         }
      }
   }

   {
      key = "http-server-cert-key";
      rapidjson::Value::MemberIterator found = section->value.FindMember(key.c_str());
      if ((found != section->value.MemberEnd()) && found->value.IsString()) {
         xi::String value = found->value.GetString();
         value.Trim();
         if (!value.IsEmpty()) {
            SetHttpServerCertKey(value.c_str());
         }
      }
   }

   return true;
}


bool Property::Load_timer(rapidjson::Document &doc)
{
   static const char *FN = "[property::Load_timer] ";

   std::string key;

   key = "timer";
   rapidjson::Value::MemberIterator section = doc.FindMember(key.c_str());
   if (section == doc.MemberEnd()) {
      WLOG(FN << "not-found " << key);
      return false;
   }


   for (int index = td::timer::NONE + 1; index < td::timer::END_OF_ENUM; ++index) {
      key = td::timer::name((td::timer::e)index);
      rapidjson::Value::MemberIterator found = section->value.FindMember(key.c_str());
      if ((found != section->value.MemberEnd()) && found->value.IsNumber()) {
         timer_value_[index] = (uint32_t) found->value.GetInt();
      }
   }

   return true;
}

bool Property::Load_service(rapidjson::Document &doc)
{
   static const char *FN = "[property::Load_service] ";

   std::string key;

   key = "service";
   rapidjson::Value::MemberIterator section = doc.FindMember(key.c_str());
   if (section == doc.MemberEnd()) {
      WLOG(FN << "not-found " << key);
      return false;
   }

   {
      key = "load-generation-plan";
      rapidjson::Value::MemberIterator found = section->value.FindMember(key.c_str());
      if ((found != section->value.MemberEnd()) && found->value.IsString()) {
         xi::String value = found->value.GetString();
         load_scenario_list_ = ParseLoadScenario(value);
      }
   }

   {
      key = "api1-url";
      rapidjson::Value::MemberIterator found = section->value.FindMember(key.c_str());
      if ((found != section->value.MemberEnd()) && found->value.IsString()) {
         test_api1_url_ = found->value.GetString();
      }
   }

   return true;
}

xi::rp::pbgid_t Property::GetTopologyId()
{
   return 0;
}

const char *Property::GetHostname()
{
   return hostname_.c_str();
}

const char *Property::GetCliID()
{
   return cli_id_.c_str();
}

const char *Property::GetCliPW()
{
   return cli_pw_.c_str();
}

uint16_t Property::GetCliPort()
{
   return cli_port_;
}

xi::loglevel::e Property::GetLogLevel()
{
   return GETLOGLEVEL();
}

void Property::SetLogLevel(xi::loglevel::e level)
{
   SETLOGLEVEL(level);
}

unsigned int Property::GetLogFileSize()
{
   return GETLOGFILESIZE() / 1024 / 1024;
}

void Property::SetLogFileSize(unsigned int filesize)
{
   SETLOGFILESIZE(filesize * 1024 * 1024);
}

unsigned int Property::GetLogFileLimit()
{
   return GETLOGFILELIMIT();
}

void Property::SetLogFileLimit(unsigned int nLimit)
{
   if (nLimit > 999) nLimit = 999; 

   SETLOGFILELIMIT(nLimit);
}

unsigned int Property::GetRpPoolSize()
{
   return rp_pool_size_;
}

uint16_t Property::GetHttpBindTcpPort()
{
   return http_bind_tcp_port_;
}

uint16_t Property::GetHttpBindTlsPort()
{
   return http_bind_tls_port_;
}

const char *Property::GetHttpServerCertificate()
{
   return http_server_certificate_.c_str();
}

const char *Property::GetHttpServerCertKey()
{
   return http_server_cert_key_.c_str();
}

void Property::SetHttpServerCertificate(const char *file)
{
   http_server_certificate_ = file;
}

void Property::SetHttpServerCertKey(const char *file)
{
   http_server_cert_key_ = file;
}

uint32_t Property::GetTimerValue(td::timer::e type)
{
   static const char *FN = "[property::GetTimerValue] ";

   if (false == td::timer::valid(type)) {
      WLOG(FN << RED(" unknown timertype:" << type));
      return 0;
   }

   // TPS를 1sec동안 고르게 발생시켜야 한다.
   if (td::timer::TRIGGER_SCENARIO == type)
      return random_() % 950;

   return timer_value_[type];
}

bool Property::SetTimerValue(td::timer::e type, uint32_t value)
{
   static const char *FN = "[property::SetTimerValue] ";

   if (false == td::timer::valid(type)) {
      WLOG(FN << RED(" unknown timertype:" << type));
      return false;
   }

   timer_value_[type] = value;

   return true;
}

std::list<test::Load> Property::GetLoadScenario()
{
   return load_scenario_list_;
}

void Property::SetLoadScenario(std::list<test::Load> &list)
{
   load_scenario_list_ = list;
}

std::list<test::Load> Property::ParseLoadScenario(xi::String &cmd_line)
{
   std::list<test::Load> load_list;

   xi::Splitter split(cmd_line.c_str(), ',');
   for (uint32_t index = 0; index < split.GetSize(); ++index)
   {
      xi::String scenario_name;
      xi::String scenario_cps;

      xi::InputStringCursor cursor(split.At(index), strlen(split.At(index) ? split.At(index) : ""));

      cursor.SkipBlank();
      cursor.ReadNonBlank(scenario_name);
      cursor.SkipBlank();
      cursor.ReadDigit(scenario_cps);

      if (scenario_name.IsEmpty() || (false == scenario_cps.IsDigit()))
         continue;

      test::Load load;
      load.scenario_ = test::scenario::code(scenario_name.c_str());
      load.turn_     = scenario_cps.AsUInt32();

      if ((test::scenario::NONE != load.scenario_) && load.turn_)
         load_list.push_back(load);
   }

   return load_list;
}

std::string Property::ToStringLoadScenario()
{
   std::string retval;

   for (std::list<test::Load>::iterator iter = load_scenario_list_.begin(); iter != load_scenario_list_.end(); ++iter)
   {
      if (iter != load_scenario_list_.begin())
         retval += ", ";

      char buffer[128];
      snprintf(buffer, sizeof(buffer), "%s %u", test::scenario::name(iter->scenario_), iter->turn_);
      retval += buffer;
   }

   return retval;
}

const char *Property::GetApi1Url() const
{
   return test_api1_url_.c_str();
}

void Property::ResetTestFlag()
{
   for (int i = 0; i < test::flag::toggle::END_OF_ENUM; ++i)
      test_flag_toggle_[i] = test::flag::toggle::default_value((test::flag::toggle::e)i);

   for (int i = 0; i < test::flag::number::END_OF_ENUM; ++i)
      test_flag_number_[i] = test::flag::number::default_value((test::flag::number::e)i);

   for (int i = (int)test::flag::letter::NONE + 1; i < (int)test::flag::letter::END_OF_ENUM; ++i)
      test_flag_letter_[i] = test::flag::letter::default_value((test::flag::letter::e) i);
}

bool Property::GetTestFlag(test::flag::toggle::e name) const
{
   static const char *FN = "[property::GetTestFlag::toggle] ";

   if ((test::flag::toggle::NONE < name) && (test::flag::toggle::END_OF_ENUM > name))
      return test_flag_toggle_[name];

   WLOG(FN << "invalid test::flag::toggle name:" << test::flag::toggle::name(name));

   return false;
}

void Property::SetTestFlag(test::flag::toggle::e name, bool value)
{
   static const char *FN = "[property::SetTestFlag::toggle] ";

   if ((test::flag::toggle::NONE < name) && (test::flag::toggle::END_OF_ENUM > name)) {
      test_flag_toggle_[name] = value;
      DLOG(FN << "test::flag::toggle name:" << test::flag::toggle::name(name) << " Value:" << (value ? "ON" : "OFF"));

   } else {
      WLOG(FN << "invalid test::flag::toggle name:" << test::flag::toggle::name(name));
   }
}

int Property::GetTestFlag(test::flag::number::e name) const
{
   static const char *FN = "[property::GetTestFlag::number] ";

   if ((test::flag::number::NONE < name) && (test::flag::number::END_OF_ENUM > name))
      return test_flag_number_[name];

   WLOG(FN << "invalid test::flag::number name:" << test::flag::number::name(name));

   return 0;
}

void Property::SetTestFlag(test::flag::number::e name, int value)
{
   static const char *FN = "[property::SetTestFlag::number] ";

   if ((test::flag::number::NONE < name) && (test::flag::number::END_OF_ENUM > name)) {
      test_flag_number_[name] = value;
      DLOG(FN << "test::flag::number name:" << test::flag::number::name(name) << " Value:" << (value ? "ON" : "OFF"));

   } else {
      WLOG(FN << "invalid test::flag::number name:" << test::flag::number::name(name));
   }
}

const char *Property::GetTestFlag(test::flag::letter::e name) const
{
   static const char *FN = "[property::GetTestFlag::letter] ";

   if ((test::flag::letter::NONE < name) && (test::flag::letter::END_OF_ENUM > name)) {
      if(test_flag_letter_[name].IsEmpty()) {
         return "";
      }
      return test_flag_letter_[name].c_str();
   }

   WLOG(FN << "invalid test::flag::letter name:" << test::flag::letter::name(name));

   return 0;
}

void Property::SetTestFlag(test::flag::letter::e name, const char *value)
{
   static const char *FN = "[property::SetTestFlag::letter] ";

   if ((test::flag::letter::NONE < name) && (test::flag::letter::END_OF_ENUM > name)) {
      test_flag_letter_[name] = value;
      DLOG(FN << "test::flag::letter name:" << test::flag::letter::name(name) << " Value:" << test_flag_letter_[name].c_log());

   } else {
      WLOG(FN << "invalid test::flag::letter name:" << test::flag::letter::name(name));
   }
}

bool Property::SameAsDefault(test::flag::toggle::e name)
{
   if (false == test::flag::toggle::scope(name))
      return false;

   if (test::flag::toggle::default_value(name) == GetTestFlag(name))
      return true;

   return false;
}

bool Property::SameAsDefault(test::flag::number::e name)
{
   if (false == test::flag::number::scope(name))
      return false;

   if (test::flag::number::default_value(name) == GetTestFlag(name))
      return true;

   return false;
}

bool Property::SameAsDefault(test::flag::letter::e name)
{
   if (false == test::flag::letter::scope(name))
      return false;

   if (xi::IsZero(test::flag::letter::default_value(name)) == xi::IsZero(GetTestFlag(name)))
      return true;

   if (xi::StrEqual(test::flag::letter::default_value(name), GetTestFlag(name)))
      return true;

   return false;
}

bool Property::NextAddr(xi::SocketAddr *out)
{
   if (conn_max_size_ > conn_size_)
      return false;

   {
      xi::RwLock::ScopedLockRead lock(conn_lock_);
      uint32_t index = conn_seqno_ % conn_size_;
      ++conn_seqno_;

      auto iter = conn_lists_.begin();
      for (; index && (iter != conn_lists_.end()); --index, iter++) {
      }

      if (iter == conn_lists_.end())
         return false;

      *out = *iter;
      return true;
   }

   return false;
}

void Property::AddAddr(const xi::SocketAddr &addr)
{
   xi::RwLock::ScopedLockWrite lock(conn_lock_);
   conn_lists_.push_back(addr);
   conn_size_ = conn_lists_.size();
}

void Property::RemoveAddr(const xi::SocketAddr &addr)
{
   xi::RwLock::ScopedLockWrite lock(conn_lock_);

   for (auto iter = conn_lists_.begin(); iter != conn_lists_.end(); iter++) {
      if (*iter == addr) {
         conn_lists_.erase(iter);
         conn_size_ = conn_lists_.size();
         return;
      }
   }
}

void Property::SetAddrLimit(uint32_t size)
{
   conn_max_size_ = size;
}

void Property::Print()
{
   EOUT("[system]");
   EOUT("  CLI port                    : " << cli_port_);
   EOUT("  CLI login                   : " << cli_id_ << "/" << cli_pw_);
   EOUT("  log.level                   : " << xi::loglevel::name(GetLogLevel()));
   EOUT("  log.max-file-size           : " << GetLogFileSize());
   EOUT("  log.max-file-archive        : " << GetLogFileLimit());
   EOUT("---------------------------------------------------");

   EOUT("[resource]");
   EOUT("  rp.pool-size                : " << GetRpPoolSize());
   EOUT("---------------------------------------------------");

   EOUT("[interface]");
   EOUT("  http-bind-tcp-port          : " << GetHttpBindTcpPort());
   EOUT("  http-bind-tls-port          : " << GetHttpBindTlsPort());
   EOUT("  http-server-certificate     : " << GetHttpServerCertificate());
   EOUT("  http-server-cert-key        : " << GetHttpServerCertKey());
   EOUT("---------------------------------------------------");

   EOUT("[timer]");
   {
      for (int i = td::timer::NONE + 1; i < td::timer::END_OF_ENUM; ++i) {
         xi::String name = td::timer::name((td::timer::e)i);

         xi::String OUTSTR;
         OUTSTR.Csnprintf(128, "  %-28s: %u", name.c_str(), GetTimerValue((td::timer::e)i));
         EOUT(OUTSTR);
      }
   }
   EOUT("---------------------------------------------------");

   EOUT("[flag]");
   {
      for (int i = test::flag::toggle::NONE + 1; i < test::flag::toggle::END_OF_ENUM; ++i) {
         xi::String name = test::flag::toggle::name((test::flag::toggle::e)i);

         xi::String OUTSTR;
         OUTSTR.Csnprintf(128, "  %-28s: %s", name.c_str(), GetTestFlag((test::flag::toggle::e)i) ? "true" : "false");
         EOUT(OUTSTR);
      }
   }
   {
      for (int i = test::flag::letter::NONE + 1; i < test::flag::letter::END_OF_ENUM; ++i) {
         xi::String name = test::flag::letter::name((test::flag::letter::e)i);

         xi::String OUTSTR;
         OUTSTR.Csnprintf(128, "  %-28s: ", name.c_str());
         EOUT(OUTSTR << GetTestFlag((test::flag::letter::e)i));
      }
   }
   EOUT("---------------------------------------------------");

   EOUT("[service]");
   EOUT("  load-generation-plan        : " << ToStringLoadScenario());
   EOUT("  api1-url                    : " << GetApi1Url());
   EOUT("---------------------------------------------------");
}


}
