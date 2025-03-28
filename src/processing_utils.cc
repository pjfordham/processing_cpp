#include "processing_utils.h"
#include <fstream>
#include <curl/curl.h>

TableRow::TableRow() {}
TableRow::TableRow(const std::map<std::string, std::string>& rowData) : data(rowData) {}

std::string TableRow::getString(const std::string& column) const {
   auto it = data.find(column);
   return it != data.end() ? it->second : "";
}

float TableRow::getFloat(const std::string& column) const {
   return atof(getString(column).c_str());
}

void TableRow::setString(const std::string& column, const std::string& value) {
   data[column] = value;
}

void TableRow::setFloat(const std::string& column, float value) {
   setString(column, fmt::format("{}", value));
}

const std::map<std::string, std::string>& TableRow::getData() const {
      return data;
}

Table::Table() {}

std::vector<TableRow> Table::rows() { return _rows; }

bool Table::loadCSV(const std::string& filename) {
   std::ifstream file(std::string("data/")+filename);
   if (!file.is_open()) {
      std::cerr << "Error opening file: " << filename << std::endl;
      return false;
   }

   std::string line;
   if (std::getline(file, line)) {
      std::stringstream ss(line);
      std::string column;
      while (std::getline(ss, column, ',')) {
         headers.push_back(column);
      }
   }
   while (std::getline(file, line)) {
      std::stringstream ss(line);
      std::string value;
      std::map<std::string, std::string> rowData;
      for (const auto& header : headers) {
         if (!std::getline(ss, value, ',')) break;
         rowData[header] = value;
      }
      _rows.emplace_back(rowData);
   }

   file.close();
   return true;
}

TableRow &Table::addRow() {
   _rows.emplace_back();
   if (_rows.size() > 10) {
      _rows.erase(_rows.begin());
   }
   return _rows.back();
}

int Table::getRowCount() const {
   return _rows.size();
}

void Table::removeRow(int index) {
   if (index >= 0 && index < _rows.size()) {
      _rows.erase(_rows.begin() + index);
   } else {
      std::cerr << "Invalid row index." << std::endl;
      }
}

bool Table::saveCSV(const std::string& filename) {
   std::ofstream file(std::string("data/") + filename);
   if (!file.is_open()) {
      std::cerr << "Error saving file: " << filename << std::endl;
      return false;
   }

   for (size_t i = 0; i < headers.size(); ++i) {
      file << headers[i] << (i < headers.size() - 1 ? "," : "\n");
   }

   for (const auto& row : _rows) {
      const auto& rowData = row.getData();
      for (size_t i = 0; i < headers.size(); ++i) {
         file << rowData.at(headers[i]) << (i < headers.size() - 1 ? "," : "\n");
      }
   }

   file.close();
   return true;
}

Table loadTable(const std::string& file, const std::string& header) {
   Table table;
   if ( header != "header" )
      abort();
   table.loadCSV(file);
   return table;
}

void saveTable(Table &t, const std::string &file ) {
   t.saveCSV(file);
}

void link(std::string_view link) {
   (void)!system(fmt::format("xdg-open {} >nul 2>nul",link).c_str());
}

// Trim leading and trailing whitespace from a string
std::string trim(const std::string& str) {
    size_t start = 0;
    size_t end = str.length();

    // Find the first non-whitespace character from the beginning
    while (start < end && std::isspace(str[start])) {
        ++start;
    }

    // Find the first non-whitespace character from the end
    while (end > start && std::isspace(str[end - 1])) {
        --end;
    }

    // Return the trimmed substring
    return str.substr(start, end - start);
}

static size_t write_callback(char* ptr, size_t size, size_t nmemb, void* userdata)
{
    size_t realsize = size * nmemb;
    auto* response = static_cast<std::vector<char>*>(userdata);
    response->insert(response->end(), ptr, ptr + realsize);
    return realsize;
}

std::vector<char> loadURL(std::string_view URL) {
   // Set up the libcurl easy handle
   std::string sURL{URL};
   CURL* curl = curl_easy_init();
   curl_easy_setopt(curl, CURLOPT_URL, sURL.c_str());
   curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_callback);
   std::vector<char> response_body;
   curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response_body);

   // Perform the request
   CURLcode res = curl_easy_perform(curl);
   curl_easy_cleanup(curl);

   if (res == CURLE_OK) {
      return response_body;
   } else {
      // If it didn't download check the local filesystem
      using namespace std::literals;
      std::ifstream file(("data/"s + sURL), std::ios::binary);
      if (file.is_open()) {
         // Read the local file into a vector
         file.seekg(0, std::ios::end);
         std::streampos file_size = file.tellg();
         file.seekg(0, std::ios::beg);

         std::vector<char> local_data(file_size);
         file.read(local_data.data(), local_data.size());
         file.close();
         return local_data;
      }
      return {};
   }
}


std::vector<std::string> loadStrings(std::string_view fileName) {
   std::vector<char> data = loadURL(fileName);
   std::vector<std::string> strings;

   auto i = data.begin();
   auto j = i;
   while (i != data.end() ) {
      while ( *(++j) != '\n' && j != data.end()) {}
      if (j == data.end()) {
          strings.push_back({i,j});
          break;
      } else {
          strings.push_back({i,++j});
      }
      i = j;
   }
   return strings;
}

void saveStrings(std::string_view fileName, std::vector<std::string> &data) {
   std::vector<std::string> lines;
   std::ofstream outputFile("data/"s.append(fileName));

   if (!outputFile.is_open()) {
      abort();
   }

   for (const std::string& line : data) {
      outputFile << line << std::endl; // Write each string followed by a newline
   }

   outputFile.close();
   return;
}

std::vector<std::string> split(std::string_view str, char delimiter) {
   std::vector<std::string> result;
   std::string token;

   for (const char& c : str) {
      if (c == delimiter) {
         result.push_back(token);
         token.clear();
      } else {
         token += c;
      }
   }

   // Push back the last token, if any
   if (!token.empty()) {
      result.push_back(token);
   }

   return result;
}

std::vector<std::string> split(std::string_view input, std::string_view delimiter) {
    std::vector<std::string> tokens;
    size_t startPos = 0;
    size_t foundPos;

    while ((foundPos = input.find(delimiter, startPos)) != std::string::npos) {
        // Extract the token from startPos to foundPos
        std::string_view token = input.substr(startPos, foundPos - startPos);

        // Add the token to the vector
        tokens.emplace_back(token);

        // Update the start position to the character after the found delimiter
        startPos = foundPos + delimiter.length();
    }

    // Add the last token (or the whole string if no delimiter is found)
    std::string_view token = input.substr(startPos);
    tokens.emplace_back(token);

    return tokens;
}

std::vector<std::string> splitTokens(std::string_view input, std::string_view delimiters) {
   std::vector<std::string> tokens;
   size_t startPos = 0;
   size_t foundPos;

   while ((foundPos = input.find_first_of(delimiters, startPos)) != std::string::npos) {
      // Extract the token from startPos to foundPos
      std::string_view token = input.substr(startPos, foundPos - startPos);

      // Add the token to the vector
      tokens.emplace_back(token);

      // Update the start position to the character after the found delimiter
      startPos = foundPos + 1;
   }

   // Add the last token (or the whole string if no delimiter is found)
   std::string_view token = input.substr(startPos);
   tokens.emplace_back(token);

   return tokens;
}

std::string join(const std::vector<std::string>& strings, std::string_view separator) {
   std::string result;

   for (size_t i = 0; i < strings.size(); ++i) {
      result += strings[i];

      // Add the separator if not the last element
      if (i < strings.size() - 1) {
         result += separator;
      }
   }

   return result;
}

std::string toLowerCase(std::string_view input) {

   std::string output(input);
   for (char &c : output) {
      c = std::tolower(c);
   }

   return output;
}

bool endsWith(std::string_view fullString, std::string_view ending) {
   if (fullString.length() >= ending.length()) {
      return (fullString.compare(fullString.length() - ending.length(), ending.length(), ending) == 0);
   } else {
      return false;
   }
}
