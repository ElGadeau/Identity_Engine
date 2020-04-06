#pragma once
#include <Export.h>
#include <Objects/IObject.h>

namespace Engine::Objects
{
    class GameObject;
}

namespace Engine::Components
{
    class API_ENGINE IComponent : public Objects::IObject
    {
    public:
        IComponent(Objects::GameObject* p_gameObject);

        virtual bool operator==(IComponent* p_other) = 0;
        virtual bool DeleteFromMemory() = 0;
        void SetGameObject(std::shared_ptr<Objects::GameObject> p_gameObject);
        [[nodiscard]] std::shared_ptr<Objects::GameObject> GetGameObject() const;

    protected:
        std::shared_ptr<Objects::GameObject> m_gameObject;
        //Objects::GameObject* m_gameObject;
    };
}