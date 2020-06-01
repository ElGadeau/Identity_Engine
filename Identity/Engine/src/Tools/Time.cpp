#include <stdafx.h>

#include <Containers/EventContainer.h>
#include <Tools/Time.h>

using namespace Engine::Tools;

Time::Time(int p_numberOfSamples = 10) : m_numberOfSamples{p_numberOfSamples}
{
    Containers::EventContainer::AddEvent("CallFixedUpdate");
}

void Time::Start()
{
    GetInstance()->m_startTime = std::chrono::high_resolution_clock::now();
}

void Time::Stop()
{
    GetInstance()->m_updateFrameTime = std::chrono::high_resolution_clock::now();
    GetInstance()->m_deltaTime       = std::chrono::duration_cast<std::chrono::milliseconds>(GetInstance()->m_endTime - GetInstance()->m_startTime).count();
    GetInstance()->m_previousTimes.push_back(GetInstance()->m_deltaTime);

    if (GetInstance()->m_previousTimes.size() > GetInstance()->m_numberOfSamples)
    {
        GetInstance()->m_previousTimes.pop_front();
    }

    GetInstance()->m_updateFrameTime        = std::chrono::high_resolution_clock::now();
    const std::chrono::duration<float> time = GetInstance()->m_updateFrameTime - GetInstance()->m_lastUpdateFrameTime;


    if (time.count() >= 1)
    {
        GetInstance()->m_lastUpdateFrameTime = std::chrono::high_resolution_clock::now();

        float totalTime = 0.0f;

        for (auto time : GetInstance()->m_previousTimes)
        {
            totalTime += time;
        }

        GetInstance()->m_FPS = totalTime / GetInstance()->m_previousTimes.size();
    }
    GetInstance()->m_endTime = std::chrono::high_resolution_clock::now();
}

void Time::Update()
{
    /* GetInstance()->m_fixedUpdateCounter += GetInstance()->m_deltaTime;
 
     if (GetInstance()->m_fixedUpdateCounter >= 1.0f)
     {
         GetInstance()->m_fixedUpdateCounter = 0.0f;
         Containers::EventContainer::GetEvent("CallFixedUpdate").Fire();
     }*/
}

int Time::GetFPS()
{
    float fps = 1.0f / GetInstance()->m_FPS * 1000.0f;

    return fps;
}

float Time::GetDeltaTime()
{
    float deltaTime = GetInstance()->m_deltaTime;

    if (1.0f / deltaTime > GetInstance()->m_fpsLimiter)
        deltaTime = 1.0f / GetInstance()->m_fpsLimiter;

    if (deltaTime < 0)
        return 0;

    return deltaTime;
}

std::unique_ptr<Time>& Time::GetInstance()
{
    if (m_instance == nullptr)
    {
        m_instance = std::make_unique<Time>(100);
    }

    return m_instance;
}