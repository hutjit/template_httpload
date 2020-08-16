// vim:ts=3:sts=3:sw=3

#ifndef TEMPLATE_HSM_LIBXML2_UTIL_H_
#define TEMPLATE_HSM_LIBXML2_UTIL_H_

#include <libxml/tree.h>
#include <libxml/parser.h>
#include <libxml/xpath.h>
#include <libxml/xpathInternals.h>

#include "xi/singleton.hxx"

namespace hs {


class Libxml2Util : public xi::Singleton<Libxml2Util>
{
   public :
      Libxml2Util();
      ~Libxml2Util();

      xmlNode *FindNextNode(xmlNode *pNode, const char *pName);
      xmlNode *GetTextNode(xmlNode *pValue);
      xmlNode *GetCDATANode(xmlNode *pValue);
      struct _xmlAttr *FindNextAttr(struct _xmlAttr *pAttr, const char *pName);
};


}

#endif
