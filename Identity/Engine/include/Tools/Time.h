#pragma once
#include <Export.h>
#include <chrono>
#include <queue>
#include <memory>

namespace Engine::Tools
{
    class API_ENGINE Time
    {
    public:
        Time(int p_numberOfSamples);
        Time(const Time&) = delete;
        Time(const Time&&) = delete;
    	
        static void Start();
        static void Stop();
        static void Update();
    	
        /**
         * @return Frames per second.
         */
        static int GetFPS();

        /***
         * @return Time taken to finish a frame, in milliseconds.
         */
        static double GetDeltaTime();
        static double GetFixedUpdateCounter() { return GetInstance()->m_fixedUpdateCounter; }
        static void SetFixedUpdateCounter(double p_counter) { GetInstance()->m_fixedUpdateCounter = p_counter; }
        //static std::chrono::time_point<std::chrono::system_clock>& GetFixedUpdate() { return GetInstance()->m_fixedUpdate; }
        static std::unique_ptr<Time>& GetInstance();
    	
    private:  	
        static inline std::unique_ptr<Time> m_instance{ nullptr };
    	
        int m_numberOfSamples;
        double m_deltaTime = 0.0;
        double m_FPS;
        double m_fpsLimiter = 125.0f;
        double m_fixedUpdateCounter = 0.0f;
    	
        std::chrono::time_point<std::chrono::system_clock> m_startTime;
        std::chrono::time_point<std::chrono::system_clock> m_endTime;

        std::chrono::time_point<std::chrono::system_clock> m_updateFrameTime;
        std::chrono::time_point<std::chrono::system_clock> m_lastUpdateFrameTime;
    	
        std::deque<double> m_previousTimes;
    };
}
