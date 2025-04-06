#ifndef PROCESSING_XML_H
#define PROCESSING_XML_H

#include <memory>
#include <string>
#include <string_view>
#include <vector>

extern "C" {
  typedef struct _xmlNode xmlNode;
  typedef struct _xmlDoc xmlDoc;
}

class XML {
public:
   std::shared_ptr<xmlDoc> doc;
   xmlNode *element;

   XML();
   XML(std::shared_ptr<xmlDoc> d, xmlNode *e);

   std::vector<XML> getChildren(std::string_view name);
   XML getChild(std::string_view name);
   XML addChild(std::string_view name);
   void removeChild(XML x);
   int getInt(std::string_view attrName);
   void setInt(std::string_view attrName, int value);
   std::string getString(std::string_view attrName);
   float getFloatContent();
   int getIntContent();
   std::string getContent();
   void setFloatContent(float f);
   void setIntContent(int f);
   void setContent(std::string content);
};

XML loadXML(std::string_view filename);
void saveXML(XML xml, std::string_view filename);

#endif
