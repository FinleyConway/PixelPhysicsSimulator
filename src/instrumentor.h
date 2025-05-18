#pragma once

#include <thread>
#include <string>
#include <chrono>
#include <fstream>

// https://ui.perfetto.dev/

using FloatingPointMicroseconds = std::chrono::duration<double, std::micro>;

struct ProfileResult
{
    std::string name;
    FloatingPointMicroseconds start;
    std::chrono::microseconds elapsedTime;
    std::thread::id threadID;
};

struct InstrumentationSession
{
    std::string name;
};

class Instrumentor
{
public:
    Instrumentor(const Instrumentor&) = delete;

    Instrumentor(Instrumentor&&) = delete;

    void BeginSession(const std::string& name, const std::string& filepath, size_t sizeBeforeDump = 2048)
    {
        m_Filepath = filepath;
        m_SizeBeforeDump = sizeBeforeDump * 1024 * 1024;

        // close a session if one is open
        if (m_CurrentSession != nullptr)
        {
            InternalEndSession();
        }

        m_OutputStream.open(filepath);

        if (m_OutputStream.is_open())
        {
            m_CurrentSession = std::make_unique<InstrumentationSession>(name);
            WriteHeader();
        }
    }

    void EndSession()
    {
        InternalEndSession();
    }

    void WriteProfile(const ProfileResult& result)
    {
        // temp solution for now
        // when the stream reaches a certain size, refresh the file
        if (m_OutputStream.is_open() && m_OutputStream.tellp() >= m_SizeBeforeDump)
        {
            m_OutputStream.close();
            m_OutputStream.open(m_Filepath, std::ios::out | std::ios::trunc);
            WriteHeader();
        }

        std::stringstream json;

        json << std::setprecision(3) << std::fixed;
        json << ",{";
        json << "\"cat\":\"function\",";
        json << "\"dur\":" << (result.elapsedTime.count()) << ',';
        json << "\"name\":\"" << result.name << "\",";
        json << "\"ph\":\"X\",";
        json << "\"pid\":0,";
        json << "\"tid\":" << result.threadID << ",";
        json << "\"ts\":" << result.start.count();
        json << "}";

        if (m_CurrentSession)
        {
            m_OutputStream << json.str();
            m_OutputStream.flush();
        }
    }

    static Instrumentor& Get()
    {
        static Instrumentor instance;
        return instance;
    }

private:
    Instrumentor() = default;

    ~Instrumentor()
    {
        EndSession();
    }

    void WriteHeader()
    {
        m_OutputStream << R"({"otherData": {},"traceEvents":[{})";
        m_OutputStream.flush();
    }

    void WriteFooter()
    {
        m_OutputStream << "]}";
        m_OutputStream.flush();
    }

    void InternalEndSession()
    {
        if (m_CurrentSession != nullptr)
        {
            WriteFooter();

            m_OutputStream.close();
            m_CurrentSession.reset();
        }
    }

private:
    std::unique_ptr<InstrumentationSession> m_CurrentSession = nullptr;
    std::string m_Filepath;
    std::ofstream m_OutputStream;
    size_t m_SizeBeforeDump = 512 * 1024 * 1024; // default of 512mb
};

class InstrumentationTimer
{
public:
    explicit InstrumentationTimer(const std::string_view name) : m_Name(name)
    {
        m_StartTimePoint = std::chrono::steady_clock::now();
    }

    ~InstrumentationTimer()
    {
        if (!m_Stopped) Stop();
    }

    void Stop()
    {
        const auto endTimePoint = std::chrono::steady_clock::now();
        const auto highResStart = FloatingPointMicroseconds(m_StartTimePoint.time_since_epoch());
        const auto elapsedTime =
            std::chrono::time_point_cast<std::chrono::microseconds>(endTimePoint).time_since_epoch() -
            std::chrono::time_point_cast<std::chrono::microseconds>(m_StartTimePoint).time_since_epoch();

        Instrumentor::Get().WriteProfile({ m_Name.data(), highResStart, elapsedTime, std::this_thread::get_id() });

        m_Stopped = true;
    }

private:
    const std::string_view m_Name;
    std::chrono::time_point<std::chrono::steady_clock> m_StartTimePoint;
    bool m_Stopped = false;
};

#define ENABLE_PROFILING 1

#ifdef ENABLE_PROFILING
    #define PROFILE_BEGIN_SESSION(name, filepath, dumpThres) Instrumentor::Get().BeginSession(name, filepath, dumpThres)
    #define PROFILE_END_SESSION() Instrumentor::Get().EndSession()

    #define PROFILE_SCOPE(name) InstrumentationTimer timer##__LINE__(name)
    #define PROFILE_FUNCTION(name) PROFILE_SCOPE(__FUNCTION__)
#else
    #define PROFILE_BEGIN_SESSION(name, filepath, dumpThres)
    #define PROFILE_END_SESSION()

    #define PROFILE_SCOPE(name)
    #define PROFILE_FUNCTION()
#endif