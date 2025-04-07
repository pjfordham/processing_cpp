#ifndef PROCESSING_JSON_H
#define PROCESSING_JSON_H

#include <memory>
#include <string>
#include <string_view>
#include <vector>
#include <nlohmann/json.hpp>

class JSONArray;

class JSONObject {
public:
   std::shared_ptr<nlohmann::json> root;
   nlohmann::json *element;

   JSONObject();
   JSONObject(std::shared_ptr<nlohmann::json> root, nlohmann::json *element);

   JSONArray getJSONArray(std::string_view name);
   JSONObject getJSONObject(std::string_view name);
   void setJSONObject(std::string_view name, const JSONObject &x);
   int getInt(std::string_view attrName);
   float getFloat(std::string_view attrName);
   void setFloat(std::string_view attrName, float x);
   void setInt(std::string_view attrName, int value);
   std::string getString(std::string_view attrName);
   void setString(std::string_view attrName, std::string_view x);
};

class JSONArray {
public:
   JSONArray(std::shared_ptr<nlohmann::json> root, nlohmann::json *array);
   int size();
   void append(const JSONObject &b);
   JSONObject getJSONObject(int i);
   void remove(int);

private:
   std::shared_ptr<nlohmann::json> root;
   nlohmann::json *array;
};

JSONObject loadJSONObject(std::string_view filename);
void saveJSONObject(const JSONObject &json, std::string_view filename);

#endif
