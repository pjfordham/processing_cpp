#include <processing_xml.h>

#include <libxml/parser.h>
#include <libxml/tree.h>
#include <memory>
#include <string>
#include <string_view>
#include <vector>


XML::XML() : doc(nullptr), element(nullptr) {}
XML::XML(std::shared_ptr<xmlDoc> d, xmlNode *e) : doc(std::move(d)), element(e) {}

std::vector<XML> XML::getChildren(std::string_view name) {
   std::vector<XML> children;
   for (xmlNode *child = element->children; child; child = child->next) {
      if (child->type == XML_ELEMENT_NODE && name == (const char *)child->name) {
         children.emplace_back(doc, child);
      }
   }
   return children;
   }

XML XML::getChild(std::string_view name) {
   for (xmlNode *child = element->children; child; child = child->next) {
      if (child->type == XML_ELEMENT_NODE && name == (const char *)child->name) {
         return XML(doc, child);
      }
   }
   return XML(doc, nullptr);
}

XML XML::addChild(std::string_view name) {
   xmlNode *child = xmlNewChild(element, nullptr, BAD_CAST std::string(name).c_str(), nullptr);
   return XML(doc, child);
}

void XML::removeChild(XML x) {
   if (x.element && x.element->parent == element) {
         xmlUnlinkNode(x.element);
         xmlFreeNode(x.element);
   }
}

int XML::getInt(std::string_view attrName) {
   xmlChar *val = xmlGetProp(element, BAD_CAST std::string(attrName).c_str());
   if (!val) return 0;
   int result = std::stoi((const char *)val);
   xmlFree(val);
   return result;
}

void XML::setInt(std::string_view attrName, int value) {
   xmlSetProp(element, BAD_CAST std::string(attrName).c_str(), BAD_CAST std::to_string(value).c_str());
}

float XML::getFloatContent() {
   xmlChar *content = xmlNodeGetContent(element);
   if (!content) return 0.0f;
   float val = std::stof((const char *)content);
   xmlFree(content);
   return val;
}

std::string XML::getContent() {
   xmlChar *content = xmlNodeGetContent(element);
   std::string result = content ? (char *)content : "";
   xmlFree(content);
   return result;
}

void XML::setFloatContent(float f) {
   std::string str = std::to_string(f);
   xmlNodeSetContent(element, BAD_CAST str.c_str());
}

void XML::setContent(std::string content) {
   xmlNodeSetContent(element, BAD_CAST content.c_str());
}

XML loadXML(std::string_view filename) {
   std::string f = std::string("data/") + std::string(filename);
   xmlDoc *doc = xmlReadFile(f.data(), nullptr, 0);
   if (!doc) return {};
   xmlNode *root = xmlDocGetRootElement(doc);
   return XML(std::shared_ptr<xmlDoc>(doc, xmlFreeDoc), root);
}

void saveXML(XML xml, std::string_view filename) {
   if (xml.doc) {
      xmlSaveFormatFileEnc(filename.data(), xml.doc.get(), "UTF-8", 1);
   }
}
