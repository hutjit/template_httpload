// vim:ts=3:sts=3:sw=3

#include "http_detail_records.h"
#include <stdio.h>
#include <stdlib.h>
#include "xi/logger.hxx"
#include "xi/pretty_table.hxx"
#include "xi/splitter.hxx"
#include "xi/util.hxx"
#include "template/main/property.h"
#include "libxml2_util.h"

namespace hs {


HttpDetailRecords::HttpDetailRecords()
{
   inject_delimiter_ = ',';
   value_size_ = 0;

   load_seq_ = 0;
}

HttpDetailRecords::~HttpDetailRecords()
{
}

bool HttpDetailRecords::Load(const char *name, const char *file)
{
   xi::RwLock::ScopedLockWrite lock(lock_);

   name_ = name;
   if (false == LoadScenario(file))
      return false;

   if (false == inject_file_.IsEmpty()) {
      if (false == LoadIngection(inject_file_.c_str()))
         return false;
   }

   return true;
}

std::string HttpDetailRecords::Summary(const uint32_t left_margin) const
{
   xi::RwLock::ScopedLockRead lock(lock_);

   xi::PrettyTable table;
   table.SetMarginLeft(left_margin);
   table.SetHead(0, "parameter");
   table.SetHead(1, "value");

   {
      std::vector<std::string> row{"scenario", name_.c_log()};
      table.AppendBody(row);
   }

   {
      std::vector<std::string> row{"http-scheme", http_scheme_.c_log()};
      table.AppendBody(row);
   }

   {
      std::vector<std::string> row{"injection-file", inject_file_.c_log()};
      table.AppendBody(row);
   }

   {
      xi::String header;
      for (auto it : header_) {
         if (header.IsEmpty()) {
            header = it;
         } else {
            header += ",";
            header += it;
         }
      }
      std::vector<std::string> row{"header", header.c_log()};
      table.AppendBody(row);
   }

   {
      xi::String rows; rows = value_size_;
      std::vector<std::string> row{"data-rows", rows.c_log()};
      table.AppendBody(row);
   }

   return table.Print();
}

bool HttpDetailRecords::Next(OUT xi::String &scheme, OUT xi::String &httpreq, OUT xi::String &content)
{
   uint32_t seq = NextSeq();

   xi::RwLock::ScopedLockRead lock(lock_);

   scheme = http_scheme_;

   // Replace에서 메모리 할당 호출을 줄이기 위해 미리 메모리를 충분히 할당함
   httpreq.Resize(1024);
   if (false == http_content_.IsEmpty())
      content.Resize(4096);

   httpreq = http_request_;
   content = http_content_;

   char varname[128];
   unsigned index = 0;
   for (auto it = header_.begin(); it != header_.end(); ++it, ++index) {
      if (it->empty())
         continue;

      xi::Splitter split(value_rows_[seq].c_str(), inject_delimiter_);
      snprintf(varname, sizeof(varname), "${%s}", it->c_str());

      httpreq.Replace(varname, split[index]);
      if (false == content.IsEmpty())
         content.Replace(varname, split[index]);
   }

   return (httpreq.IsEmpty() ? false : true);
}

bool HttpDetailRecords::LoadScenario(const char *file)
{
   const char *FN = "[HDR::LoadScenario] ";

   if (xi::IsZero(file)) {
      WLOG(FN << "invalid file(null)");
      return false;
   }

   xmlDocPtr xmldoc = xmlParseFile(file);
   if (NULL == xmldoc) {
      WLOG(FN << "xmlParseFile() fail. file:" << file);
      return false;
   }

   bool parse_status = true;
   xmlNode *root_element = xmlDocGetRootElement(xmldoc);

   // <request>
   while (parse_status) {
      xi::String section = "request";
      xmlNode *xmlnode = Libxml2Util::Instance()->FindNextNode(root_element->children, section.c_str());
      if (NULL == xmlnode) {
         WLOG(FN << "not-found children[" << section << "] of Root");
         parse_status = false;
         break;
      }

      // <scheme>
      {
         xi::String node_name = "scheme";
         xmlNode *found = Libxml2Util::Instance()->FindNextNode(xmlnode->children, node_name.c_str());
         if (NULL == found) {
            WLOG(FN << "not-found children[" << node_name << "] of " << section);
            parse_status = false;
            break;
         }
         found = Libxml2Util::Instance()->GetTextNode(found);
         if (NULL == found) {
            WLOG(FN << "not-found TEXT[" << node_name << "]");
            parse_status = false;
            break;
         }
         http_scheme_ = (const char *) found->content;
      }

      // <header>
      {
         xi::String node_name = "header";
         xmlNode *found = Libxml2Util::Instance()->FindNextNode(xmlnode->children, node_name.c_str());
         if (NULL == found) {
            WLOG(FN << "not-found children[" << node_name << "] of " << section);
            parse_status = false;
            break;
         }

         // found = Libxml2Util::Instance()->GetTextNode(found);
         found = Libxml2Util::Instance()->GetCDATANode(found);
         if (NULL == found) {
            WLOG(FN << "not-found CDATA[" << node_name << "]");
            parse_status = false;
            break;
         }
         http_request_ = (char *) found->content;
         http_request_.Trim();
         http_request_.Replace("\n", "\r\n");
         http_request_ += "\r\n";
      }

      // <content>
      {
         xi::String node_name = "content";
         xmlNode *found = Libxml2Util::Instance()->FindNextNode(xmlnode->children, node_name.c_str());
         if (NULL == found) {
            WLOG(FN << "not-found children[" << node_name << "] of " << section);
            parse_status = false;
            break;
         }

         // found = Libxml2Util::Instance()->GetTextNode(found);
         found = Libxml2Util::Instance()->GetCDATANode(found);
         if (found) {
            http_content_ = (char *) found->content;
         }
      }

      break; // loop는 1번만 실행후 종료한다.
   }

   // <injection>
   while (parse_status) {
      xi::String section = "injection";
      xmlNode *xmlnode = Libxml2Util::Instance()->FindNextNode(root_element->children, section.c_str());
      if (NULL == xmlnode) {
         WLOG(FN << "not-found children[" << section << "] of Root");
         parse_status = false;
         break;
      }

      // <delimiter>
      {
         xi::String node_name = "delimiter";
         xmlNode *found = Libxml2Util::Instance()->FindNextNode(xmlnode->children, node_name.c_str());
         if (NULL == found) {
            WLOG(FN << "not-found children[" << node_name << "] of " << section);
            parse_status = false;
            break;
         }
         found = Libxml2Util::Instance()->GetTextNode(found);
         if (NULL == found) {
            WLOG(FN << "not-found TEXT[" << node_name << "]");
            parse_status = false;
            break;
         }
         xi::String conv = (const char *) found->content;
         if (conv.GetSize()) {
            inject_delimiter_ = conv.c_str()[0];
         } else {
            WLOG(FN << "invalid " << node_name);
            parse_status = false;
            break;
         }
      }

      // <file>
      {
         xi::String node_name = "file";
         xmlNode *found = Libxml2Util::Instance()->FindNextNode(xmlnode->children, node_name.c_str());
         if (NULL == found) {
            WLOG(FN << "not-found children[" << node_name << "] of " << section);
            parse_status = false;
            break;
         }
         found = Libxml2Util::Instance()->GetTextNode(found);
         if (found) {
            inject_file_ = (const char *) found->content;
         }
      }

      break; // loop는 1번만 실행후 종료한다.
   }


   xmlFreeDoc(xmldoc);

   return parse_status;
}

bool HttpDetailRecords::LoadIngection(const char *file)
{
   const char *FN = "[HDR::LoadIngection] ";

   if (xi::IsZero(file)) {
      WLOG(FN << "invalid file(null)");
      return false;
   }

   FILE *fp = fopen(file, "r");
   if (NULL == fp) {
      WLOG(FN << "open fail. file[" << file << ")");
      return false;
   }

   bool parse_status = true;
   uint32_t column_size = 0;
   unsigned int index = 0;

   char line[4096] = {0, };
   char conv[4096] = {0, };
   while (fgets(line, sizeof(line), fp)) {
      ++index;
      if (xi::IsZero(line))
         continue;

      // header
      if (1 == index) {
         xi::Splitter split(line, inject_delimiter_);
         for (uint32_t i = 0; i < split.GetSize(); ++i) {
            if (xi::IsZero(split[i])) {
               header_.push_back("");
            } else {
               snprintf(conv, sizeof(conv), "%s", split[i]);
               xi::TrimBlank(conv);
               header_.push_back(conv);
            }
         }

         column_size = header_.size();
         continue;
      }

      // data
      {
         xi::Splitter split(line, inject_delimiter_);

         if (column_size != split.GetSize()) {
            WLOG(FN << "invalid column size header[" << column_size << "] != value[" << split.GetSize() << "], line:" << index);
            parse_status = false;
            continue;
         }
         xi::TrimBlank(line);
         value_rows_.push_back(line);
      }
   }
   value_size_ = value_rows_.size();

   fclose(fp);

   return parse_status;
}

uint32_t HttpDetailRecords::NextSeq()
{
   if (0 == value_size_)
      return 0;

   uint32_t next = (load_seq_++) % value_size_;
   return next;
}


}
