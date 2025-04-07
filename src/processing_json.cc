#include <processing_utils.h>
#include <processing_json.h>
#include <fstream>
#include <stdexcept>

using json = nlohmann::json;

// --- JSONObject ---

JSONObject::JSONObject()
   : root(std::make_shared<json>()), element(root.get()) {}

JSONObject::JSONObject(std::shared_ptr<json> r, json *e)
   : root(std::move(r)), element(e) {}

JSONArray JSONObject::getJSONArray(std::string_view name) {
   return JSONArray{root, &(*element)[std::string(name)]};
}

JSONObject JSONObject::getJSONObject(std::string_view name) {
   return JSONObject{root, &(*element)[std::string(name)]};
}

void JSONObject::setJSONObject(std::string_view name, const JSONObject &x) {
   (*element)[std::string(name)] = *x.element;
}

int JSONObject::getInt(std::string_view attrName) {
   return (*element)[std::string(attrName)].get<int>();
}

float JSONObject::getFloat(std::string_view attrName) {
   return (*element)[std::string(attrName)].get<float>();
}

void JSONObject::setFloat(std::string_view attrName, float x) {
   (*element)[std::string(attrName)] = x;
}

void JSONObject::setInt(std::string_view attrName, int value) {
   (*element)[std::string(attrName)] = value;
}

std::string JSONObject::getString(std::string_view attrName) {
   return (*element)[std::string(attrName)].get<std::string>();
}

void JSONObject::setString(std::string_view attrName, std::string_view x) {
   (*element)[std::string(attrName)] = std::string(x);
}

// --- JSONArray ---

JSONArray::JSONArray(std::shared_ptr<json> r, json *arr)
   : root(std::move(r)), array(arr) {}

int JSONArray::size() {
   return static_cast<int>(array->size());
}

void JSONArray::append(const JSONObject &b) {
   array->push_back(*b.element);
}

JSONObject JSONArray::getJSONObject(int i) {
   return JSONObject{root, &(*array)[i]};
}

void JSONArray::remove(int index) {
   array->erase(array->begin() + index);
}

// --- File I/O ---

JSONObject loadJSONObject(std::string_view filename) {
   std::vector<char> buffer = loadURL(filename); // Load raw JSON

   auto root = std::make_shared<json>(json::parse(buffer.begin(), buffer.end()));
   return JSONObject{root, root.get()};
}

void saveJSONObject(const JSONObject &jsonObj, std::string_view filename) {
   std::ofstream file{std::string(filename)};
   if (!file) throw std::runtime_error("Failed to write to file");

   file << jsonObj.element->dump(4);
}
