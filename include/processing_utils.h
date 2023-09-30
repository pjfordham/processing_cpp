#ifndef PROCESSING_UTILS_H
#define PROCESSING_UTILS_H

#include <string>
#include <string_view>
#include <vector>
#include <fmt/core.h>
#include <map>
#include <algorithm>

// This macro pulls the API a subobject into current scope.
#define MAKE_GLOBAL(method, instance) template<typename... Args> auto method(Args&&... args) { return instance.method(args...); }

using namespace std::literals;

const char TAB = '\t';

void link(std::string_view link);
std::vector<std::string> split(std::string_view str, char delimiter);
std::vector<std::string> loadStrings(std::string_view fileName);
std::vector<std::string> splitTokens(std::string_view input, std::string_view delimiters);
std::string join(const std::vector<std::string>& strings, std::string_view separator);
std::string toLowerCase(std::string_view input);


template <typename T>
std::string nf(T num, int digits) {
   return fmt::format("{0:0{1}}", num, digits);
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

#endif
