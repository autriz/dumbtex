#pragma once

#define cat_(a, b) a##b
#define cat(a, b) cat_(a, b)

#if defined(DEBUG) || defined(PROFILER)

#include <chrono>
#include <fstream>

struct ProfileResult
{
    std::string Name;
    long long Start, End;
    uint32_t ThreadID;
};

struct InstrumentationSession
{
    std::string Name;
};

class Instrumentor
{

    private:
        InstrumentationSession* m_CurrentSession;
        std::ofstream m_OutStream;
        int m_ProfileCount;

    public:
        Instrumentor() : m_CurrentSession(nullptr), m_ProfileCount(0) { }

        void BeginSession(const std::string& name, const std::string& filepath = "results.json")
        {
            m_OutStream.open(filepath);
            WriteHeader();
            m_CurrentSession = new InstrumentationSession{ name };
        }

        void EndSession()
        {
            WriteFooter();
            m_OutStream.close();
            delete m_CurrentSession;
            m_CurrentSession = nullptr;
            m_ProfileCount = 0;
        }

        void WriteProfile(const ProfileResult& result)
        {
            if (m_ProfileCount++ > 0)
                m_OutStream << ",\n\t\t";

            std::string name = result.Name;
            std::replace(name.begin(), name.end(), '"', '\'');

            m_OutStream << "{\n\t\t\t";
            m_OutStream << "\"cat\": \"function\",\n\t\t\t";
            m_OutStream << "\"dur\": " << (result.End - result.Start) << ",\n\t\t\t";
            m_OutStream << "\"name\": \"" << name << "\",\n\t\t\t";
            m_OutStream << "\"ph\": \"X\",\n\t\t\t";
            m_OutStream << "\"pid\": 0,\n\t\t\t";
            m_OutStream << "\"tid\": " << result.ThreadID << ",\n\t\t\t";
            m_OutStream << "\"ts\": " << result.Start << "\n\t\t";
            m_OutStream << "}";

            m_OutStream.flush();
        }

        void WriteHeader()
        {
            m_OutStream << "{\n\t\"otherData\": {},\n\t\"traceEvents\": [\n\t\t";
            m_OutStream.flush();
        }

        void WriteFooter()
        {
            m_OutStream << "\n\t]\n}";
            m_OutStream.flush();
        }

        static Instrumentor& Get()
        {
            static Instrumentor instance;
            return instance;
        }

};

class Timer
{
    
    public:
        Timer() : m_StartTimepoint(std::chrono::high_resolution_clock::now()), m_IsStopped(false) { m_Name = "Timer"; }
        
        Timer(const char* timer_name) : Timer()
        {
            m_Name = timer_name;
        }

        ~Timer()
        {
            if (!m_IsStopped)
                Stop();
        }

        void Stop()
        {
            auto endTimepoint = std::chrono::high_resolution_clock::now();

            auto start = std::chrono::time_point_cast<std::chrono::microseconds>(m_StartTimepoint).time_since_epoch().count();
            auto end = std::chrono::time_point_cast<std::chrono::microseconds>(endTimepoint).time_since_epoch().count();

            auto duration = end - start;
            double ms = duration * .001;

            // printf_s("[%s] execution time: %ld us (%lf ms)\n", m_Name, duration, ms);

            Instrumentor::Get().WriteProfile({ m_Name, start, end });

            m_IsStopped = true;

        }

    private:
        Timer(const Timer& other);

        std::chrono::time_point<std::chrono::high_resolution_clock> m_StartTimepoint;
        std::string m_Name;

        bool m_IsStopped;

};

#define PROFILE_SCOPE(name) Timer cat(timer, __LINE__) (name)
#define PROFILE_FUNCTION() PROFILE_SCOPE(__FUNCTION__)

#else

#define PROFILE_SCOPE(name)
#define PROFILE_FUNCTION()

#endif