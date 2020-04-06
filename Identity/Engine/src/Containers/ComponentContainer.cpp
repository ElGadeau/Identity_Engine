#include <stdafx.h>
#include <typeinfo>
#include <windows.h>
#include <Containers/ComponentContainer.h>
#include <Tools/IDCounter.h>
#include "Containers/EventContainer.h"

Engine::Containers::ComponentContainer::~ComponentContainer()
{
    delete m_instance;
}

int32_t Engine::Containers::ComponentContainer::AddComponent(Components::IComponent* p_component)
{
    for (auto& component : GetInstance()->m_components)
    {
        if (typeid(*component.second) == typeid(*p_component))
        {
            //TODO: Not the right way to compare
            if (*component.second == p_component)
            {
                std::string type = typeid(*p_component).name();

                if (component.first == p_component->GetID())
                {
                    const std::string error("ComponentContainer::AddComponent<" + type + ">(Components::IComponent* p_component): Tried to add a Component that already exists");
                    MessageBox(nullptr, error.c_str(), "Error", MB_ICONWARNING | MB_OK);
                    return component.first;
                }
            }
        }
    }

    GetInstance()->m_components.insert_or_assign(p_component->GetID(), std::shared_ptr<Engine::Components::IComponent>(p_component));
    return p_component->GetID();
}

void Engine::Containers::ComponentContainer::RemoveComponent(int32_t p_id, bool p_deleteFromMemory)
{
    if (p_deleteFromMemory)
    {
        GetInstance()->m_components.at(p_id)->DeleteFromMemory();
    }

    GetInstance()->m_components.erase(p_id);
}

Engine::Containers::ComponentContainer* Engine::Containers::ComponentContainer::GetInstance()
{
    if (m_instance == nullptr)
    {
        m_instance = new ComponentContainer();
    }

    return m_instance;
}

std::shared_ptr<Engine::Components::IComponent> Engine::Containers::ComponentContainer::FindComponent(int32_t p_id)
{
    if (GetAllComponents().find(p_id) == GetAllComponents().end())
    {
        const std::string error("ComponentContainer::FindComponent(int32_t p_id): could not find Component with ID " + p_id);
        MessageBox(nullptr, error.c_str(), "Error", MB_ICONWARNING | MB_OK);
        return nullptr;
    }

    return GetAllComponents().at(p_id);
}