#ifndef PROCESSING_PROFILE
#define PROCESSING_PROFILE

#include <string>
#include <chrono>
#include <fstream>
#include <thread>

namespace Profile {

   struct Result
   {
      std::string Name;
      long long Start, End;
      size_t ThreadID;
   };

   inline long long getTime() {
      return std::chrono::time_point_cast<std::chrono::microseconds>(std::chrono::high_resolution_clock::now()).time_since_epoch().count();
   }

   inline std::size_t getThreadID() {
      return std::hash<std::thread::id>{}(std::this_thread::get_id());
   }

   class Instrumentor
   {
   private:
      std::ofstream m_OutputStream;
      int m_ProfileCount;
      long long m_Start;
      std::string m_Name;

      Instrumentor() {
      }

   public:
      void BeginSession(const std::string& name, const std::string& filepath = "profile.json") {
         m_ProfileCount = 0;
         m_Name = name;
         m_OutputStream.open(filepath);
         WriteHeader();
         m_Start = getTime();
      }

      void EndSession() {
         Instrumentor::Get().WriteProfile({ m_Name, m_Start, getTime(), getThreadID() });
         WriteFooter();
         m_OutputStream.close();
      }

      void WriteProfile(const Result& result) {
         if (m_ProfileCount++ > 0)
            m_OutputStream << ",";

         std::string name = result.Name;
         std::replace(name.begin(), name.end(), '"', '\'');

         m_OutputStream << "{";
         m_OutputStream << "\"cat\":\"function\",";
         m_OutputStream << "\"dur\":" << (result.End - result.Start) << ',';
         m_OutputStream << "\"name\":\"" << name << "\",";
         m_OutputStream << "\"ph\":\"X\",";
         m_OutputStream << "\"pid\":0,";
         m_OutputStream << "\"tid\":" << result.ThreadID << ",";
         m_OutputStream << "\"ts\":" << result.Start;
         m_OutputStream << "}";

         m_OutputStream.flush();
      }

      void WriteHeader() {
         m_OutputStream << "{\"otherData\": {},\"traceEvents\":[";
         m_OutputStream.flush();
      }

      void WriteFooter() {
         m_OutputStream << "]}";
         m_OutputStream.flush();
      }

      static Instrumentor& Get() {
         static Instrumentor instance;
         return instance;
      }
   };

   class InstrumentationTimer
   {
   public:
      explicit InstrumentationTimer(const char* name)
         : m_Name(name) , m_Start( getTime() ) {
      }

      ~InstrumentationTimer() {
         Instrumentor::Get().WriteProfile({ m_Name, m_Start, getTime(), getThreadID() });
      }

   private:
      const char* m_Name;
      long long m_Start;
   };

} // namespace Profile

#define PROFILING 1
#if PROFILING
#define PROFILE_SCOPE(name) Profile::InstrumentationTimer timer##__LINE__(name)
#else
#define PROFILE_SCOPE(name)
#endif
#define PROFILE_FUNCTION() PROFILE_SCOPE(__PRETTY_FUNCTION__);

#endif
