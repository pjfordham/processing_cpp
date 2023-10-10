#include "processing_utils.h"
#include <fstream>
#include <curl/curl.h>

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
