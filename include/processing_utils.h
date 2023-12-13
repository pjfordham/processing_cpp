#ifndef PROCESSING_UTILS_H
#define PROCESSING_UTILS_H

#include <string>
#include <string_view>
#include <vector>
#include <fmt/core.h>
#include <map>
#include <algorithm>
#include <filesystem>
#include <chrono>
#include <ctime>
#include <fstream>


// This macro pulls the API a subobject into current scope.
#define MAKE_GLOBAL(method, instance) template<typename... Args> auto method(Args&&... args) { return instance.method(args...); }

using namespace std::literals;

const char TAB = '\t';

template <typename T>
void hash_combine(std::size_t& seed, const T& val) {
   seed ^= std::hash<T>{}(val) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
}

void link(std::string_view link);
std::vector<std::string> split(std::string_view str, char delimiter);
std::vector<std::string> loadStrings(std::string_view fileName);
void saveStrings(std::string_view fileName, std::vector<std::string> &data);
std::vector<std::string> splitTokens(std::string_view input, std::string_view delimiters);
std::string join(const std::vector<std::string>& strings, std::string_view separator);
std::string toLowerCase(std::string_view input);

std::string trim(const std::string& str);
std::vector<std::string> split(std::string_view input, std::string_view delimiter);
bool endsWith(std::string_view fullString, std::string_view ending);

inline bool startsWith(std::string_view str, std::string_view prefix) {
    if (str.size() < prefix.size()) {
        return false;
    }
    return str.substr(0, prefix.size()) == prefix;
}

inline std::string substr(std::string_view str, size_t start, size_t end) {
   if (start >= str.size() || end >= str.size()) {
      return ""; // If the position is out of bounds, return an empty string_view
   }
   return std::string(str.data() + start, end-start);
}

class PWriter {
   std::ofstream output;

 public:
   PWriter() {}

   PWriter(std::string_view filename) :
      output("data/"s.append(filename)) {
   }

   void println(std::string_view data) {
      output << data << "\n";
   }

   void close() {
      output.close();
   }

   void flush() {
      output.flush();
   }
};

inline PWriter createWriter(std::string_view name) {
   return {name};
}

template <typename T>
std::string nf(T num, int digits) {
   return fmt::format("{0:0{1}}", num, digits);
}

inline std::string nf(float num, int left, int right) {
   return fmt::format("{0:0{1}.{2}f}", num, left, right);
}

template <typename T>
std::vector<T> subset(const std::vector<T> &c, int start, int length) {
   return { c.begin() + start, c.begin() + start + length };
}

template<typename T>
void printArray(const std::vector<T> &data) {
   int i = 0;
   for ( auto &&item : data ) {
      fmt::print("[{}] {}\n",i++,item);
   }
}

template<typename KEY, typename VALUE>
class PDict {
   std::vector<KEY> m_keys;
   std::vector<VALUE> m_values;

public:
   PDict() {}

   VALUE& operator[](const KEY &key) {
      auto i = std::find(m_keys.begin(), m_keys.end(), key );
      if ( i != m_keys.end() ) {
         return m_values[ i - m_keys.begin() ];
      } else {
         m_keys.push_back(key);
         m_values.emplace_back();
         return m_values.back();
      }
   }

   VALUE& get(const KEY &key) {
      //  Return a value for the specified key
      return operator[](key);
   }

   auto size() const {
      // Returns the number of key/value pairs
      return m_values.size();
   }

   auto clear() {
      // Remove all entries from the data structure
      m_keys.clear();
      m_values.clear();
   }

   auto& keys() {
      // Return the internal array being used to store the keys
      return m_keys;
   }

   auto keyArray() const {
      // Return a copy of the internal keys array
      return m_keys;
   }

   auto& values() {
      // Return the internal array being used to store the values
      return m_values;
   }

   auto valueArray() const {
      // Create a new array and copy each of the values into it
      return m_values;
   }

   void increment(const KEY &key) {
      //   Increase the value of a specific key value by 1
      get(key)++;
   }

   void sortKey(std::function<bool(const KEY&, const KEY&)> comparator) {
      // Create an index array for sorting
      std::vector<size_t> indices(m_keys.size());
      for (size_t i = 0; i < indices.size(); ++i) {
         indices[i] = i;
      }

      // Sort the indices based on the provided comparator
      std::sort(indices.begin(), indices.end(), [&](size_t i, size_t j) {
         return comparator(m_keys[i], m_keys[j]);
      });

      // Reorder the keys and values based on sorted indices
      std::vector<KEY> sortedKeys(m_keys.size());
      std::vector<VALUE> sortedValues(m_values.size());

      for (size_t i = 0; i < indices.size(); ++i) {
         size_t originalIndex = indices[i];
         sortedKeys[i] = m_keys[originalIndex];
         sortedValues[i] = m_values[originalIndex];
      }

      // Update the original keys and values with sorted data
      m_keys = sortedKeys;
      m_values = sortedValues;
   }

   void sortValue(std::function<bool(const VALUE&, const VALUE&)> comparator) {
      // Create an index array for sorting
      std::vector<size_t> indices(m_keys.size());
      for (size_t i = 0; i < indices.size(); ++i) {
         indices[i] = i;
      }

      // Sort the indices based on the provided comparator
      std::sort(indices.begin(), indices.end(), [&](size_t i, size_t j) {
         return comparator(m_values[i], m_values[j]);
      });

      // Reorder the keys and values based on sorted indices
      std::vector<KEY> sortedKeys(m_keys.size());
      std::vector<VALUE> sortedValues(m_values.size());

      for (size_t i = 0; i < indices.size(); ++i) {
         size_t originalIndex = indices[i];
         sortedKeys[i] = m_keys[originalIndex];
         sortedValues[i] = m_values[originalIndex];
      }

      // Update the original keys and values with sorted data
      m_keys = sortedKeys;
      m_values = sortedValues;
   }

   // Sort in ascending order based on keys
   void sortKeys() {
      sortKey([](const KEY& a, const KEY& b) {
         return a < b;
      });
   }

   // Sort in descending order based on keys
   void sortKeysReverse() {
      sortKey([](const KEY& a, const KEY& b) {
         return a > b;
      });
   }
   // Sort in ascending order based on keys
   void sortValues() {
      sortValue([](const VALUE& a, const VALUE& b) {
         return a < b;
      });
   }

   // Sort in descending order based on keys
   void sortValuesReverse() {
      sortValue([](const VALUE& a, const VALUE& b) {
         return a > b;
      });
   }

};

typedef PDict<std::string, int> IntDict;
typedef PDict<std::string, float> FloatDict;

template <typename TP>
std::time_t to_time_t(TP tp) {
   using namespace std::chrono;
   auto sctp = time_point_cast<system_clock::duration>(tp - TP::clock::now()
                                                       + system_clock::now());
   return system_clock::to_time_t(sctp);
}

class File {
   std::filesystem::path path;

public:
   File(std::string name) { path = name; }

   std::time_t lastModified() const {
      return to_time_t(std::filesystem::last_write_time(path));
   }

   std::string getName() const {
      return path.filename();
   }

   std::string getAbsolutePath() const {
      return absolute( path );
   }

   bool isDirectory() const {
      return is_directory(path);
   }

   int length() const {
      try {
         return file_size(path);
      } catch(...) {
         return 0;
      }
   }

   std::vector<std::string> list() const {
      try {
         std::vector<std::string> files;
         for (const auto& entry : std::filesystem::directory_iterator(path)) {
            files.push_back(entry.path());
         }
         return files;
      }
      catch(...) {
         return {};
      }
   }

   std::vector<File> listFiles() const {
      try {
         std::vector<File> files;
         for (const auto& entry : std::filesystem::directory_iterator(path)) {
            files.push_back(File(entry.path()));
         }
         return files;
      }
      catch(...) {
         return {};
      }
   }

};

inline std::string sketchPath() { return File(".").getAbsolutePath(); }

#endif
