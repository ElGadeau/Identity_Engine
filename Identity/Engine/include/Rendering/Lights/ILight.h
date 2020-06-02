#pragma once
#include <Export.h>
#include <Objects/IObject.h>

namespace Engine::Rendering::Lights
{
    class API_ENGINE ILight: public Engine::Objects::IObject
    {
    public:
        struct LightData
        {
            Vector4F position{ 0.0f,0.0f,0.0f,0.0f};
            Vector4F ambient{ 0.1f,0.1f,0.1f,0.1f };
            Vector4F diffuse{ 1.0f,1.0f,1.0f,0.1f };
            Vector4F specular{ 1.0f,1.0f,1.0f,0.1f };
            float shininess{8.0f};
            float range{20.0f};
            float intensity{1.0f};
            float set{0.0f};
        };
        ILight(LightData p_lightData) : m_lightData{ p_lightData }{}

        LightData& GetLightData() { return m_lightData; }

        virtual void Serialize(std::ostream& p_stream) = 0;
        virtual void Deserialize(std::vector<std::string>& p_block) = 0;

    private:
        virtual bool operator==(Rendering::Lights::ILight* p_other) = 0;

        LightData m_lightData{};
    };
}
