#pragma once
#include <Export.h>
#include <3DLoader/ObjectElements/Transform.h>

namespace Engine::Objects
{
    class API_ENGINE GameObject
    {
    public:
        GameObject();
        ~GameObject();

        ObjectElements::Transform& GetTransform() { return m_transform; }

    private:
        std::shared_ptr<ObjectElements::Transform> m_transform;

    };
}