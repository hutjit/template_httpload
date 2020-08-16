// vim:ts=3:sts=3:sw=3

#include "libxml2_util.h"
#include <string.h>

namespace hs {


Libxml2Util::Libxml2Util()
{
   xmlInitParser();
}

Libxml2Util::~Libxml2Util()
{
   xmlCleanupParser();
}

xmlNode *Libxml2Util::FindNextNode(xmlNode *pNode, const char *pName)
{
   for (; pNode; pNode = pNode->next) {
      if ((XML_ELEMENT_NODE == pNode->type) && (!strcmp(pName, (const char*) pNode->name)))
         break;
   }
   return pNode;
}

xmlNode *Libxml2Util::GetTextNode(xmlNode *pValue)
{
   if (!pValue) return NULL;

   xmlNode *pNode = pValue->children;
   for (; pNode; pNode = pNode->next) {
      if (XML_TEXT_NODE == pNode->type)
         break;
   }

   return pNode;
}

xmlNode *Libxml2Util::GetCDATANode(xmlNode *pValue)
{
   if (!pValue) return NULL;

   xmlNode *pNode = pValue->children;
   for (; pNode; pNode = pNode->next) {
      if (XML_CDATA_SECTION_NODE == pNode->type)
         break;
   }

   return pNode;
}

struct _xmlAttr *Libxml2Util::FindNextAttr(struct _xmlAttr *pAttr, const char *pName)
{
   for (; pAttr; pAttr = pAttr->next) {
      if (!strcmp(pName, (const char*) pAttr->name))
         break;
   }
   return pAttr;
}


}
