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
    GetInstance()->m_startTime = std::chrono::system_clock::now();
}

void Time::Stop()
{

    GetInstance()->m_updateFrameTime = std::chrono::system_clock::now();
    

    GetInstance()->m_previousTimes.push_back(GetInstance()->m_deltaTime);

    if (GetInstance()->m_previousTimes.size() > GetInstance()->m_numberOfSamples)
    {
        GetInstance()->m_previousTimes.pop_front();
    }

    const std::chrono::duration<double> timeStamp = GetInstance()->m_updateFrameTime - GetInstance()->m_lastUpdateFrameTime;


    if (timeStamp.count() >= 1)
    {
        GetInstance()->m_lastUpdateFrameTime = std::chrono::system_clock::now();

        double totalTime = 0.0;

        for (auto time : GetInstance()->m_previousTimes)
        {
            totalTime += time;
        }

        GetInstance()->m_FPS = totalTime / GetInstance()->m_previousTimes.size();
    }
    GetInstance()->m_endTime = std::chrono::system_clock::now();
    GetInstance()->m_deltaTime = std::chrono::duration_cast<std::chrono::milliseconds>(GetInstance()->m_endTime - GetInstance()->m_startTime).count() * 0.001;
}

void Time::Update()
{
}

int Time::GetFPS()
{
    return static_cast<int>(1.0 / GetInstance()->m_FPS);
}

double Time::GetDeltaTime()
{
    double deltaTime = GetInstance()->m_deltaTime;

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
