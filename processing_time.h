#ifndef PROCESSING_TIME_H
#define PROCESSING_TIME_H

#include <chrono>

auto start_time = std::chrono::high_resolution_clock::now();

unsigned long millis() {
   auto current_time = std::chrono::high_resolution_clock::now();
   auto elapsed_time = std::chrono::duration_cast<std::chrono::milliseconds>(current_time - start_time).count();
   return elapsed_time;
}

int second() {
   // Get the current wall clock time
   auto now = std::chrono::system_clock::now();

   // Convert to time_t and then to tm structure in local time
   std::time_t time = std::chrono::system_clock::to_time_t(now);
   std::tm local_time = *std::localtime(&time);

   // Extract hours, minutes, and seconds from the tm structure
   int hours = local_time.tm_hour;
   int minutes = local_time.tm_min;
   int seconds = local_time.tm_sec;

   return seconds;
}

int minute() {
   // Get the current wall clock time
   auto now = std::chrono::system_clock::now();

   // Convert to time_t and then to tm structure in local time
   std::time_t time = std::chrono::system_clock::to_time_t(now);
   std::tm local_time = *std::localtime(&time);

   // Extract hours, minutes, and seconds from the tm structure
   int hours = local_time.tm_hour;
   int minutes = local_time.tm_min;
   int seconds = local_time.tm_sec;

   return minutes;
}

int hour() {
   // Get the current wall clock time
   auto now = std::chrono::system_clock::now();

   // Convert to time_t and then to tm structure in local time
   std::time_t time = std::chrono::system_clock::to_time_t(now);
   std::tm local_time = *std::localtime(&time);

   // Extract hours, minutes, and seconds from the tm structure
   int hours = local_time.tm_hour;
   int minutes = local_time.tm_min;
   int seconds = local_time.tm_sec;

   return hours;
}

#endif
