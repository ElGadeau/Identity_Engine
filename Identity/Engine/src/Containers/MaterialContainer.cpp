#include <stdafx.h>

#include <Containers/MaterialContainer.h>

Engine::Containers::MaterialContainer::~MaterialContainer()
{
    delete m_instance;
}

Engine::Containers::MaterialContainer* Engine::Containers::MaterialContainer::GetInstance()
{
    if (m_instance == nullptr)
    {
        m_instance = new MaterialContainer();
    }

    return m_instance;
}

int32_t Engine::Containers::MaterialContainer::AddMaterial(Rendering::Materials::Material* p_material)
{
    int32_t id = p_material->GetID();
    GetInstance()->m_material.insert_or_assign(id, std::make_shared<Rendering::Materials::Material>(*p_material));

    return id;
}

int32_t Engine::Containers::MaterialContainer::AddMaterial(const std::string& p_name)
{
    std::shared_ptr<Rendering::Materials::Material> material = std::make_shared<Rendering::Materials::Material>();
    material->SetName(p_name);
    int32_t id = material->GetID();
    GetInstance()->m_material.insert_or_assign(id, material);
    return id;
}

bool Engine::Containers::MaterialContainer::RemoveMaterial(int32_t p_id)
{
    size_t sizeBefore = GetInstance()->m_material.size();
    GetInstance()->m_material.erase(p_id);
    size_t sizeAfter = GetInstance()->m_material.size();

    return (sizeBefore != sizeAfter);
}

int32_t Engine::Containers::MaterialContainer::FindMaterial(const std::string& p_name)
{
    for (const auto& material : GetInstance()->GetAllMaterials())
    {
        if (material.second->GetName() == p_name)
            return material.first;
    }

    return -1;
}

std::shared_ptr<Engine::Rendering::Materials::Material> Engine::Containers::MaterialContainer::GetMaterial(int32_t p_id)
{
    if (GetInstance()->m_material.find(p_id) != GetInstance()->m_material.end())
        return GetInstance()->m_material.at(p_id);

    return nullptr;
}

std::shared_ptr<Engine::Rendering::Materials::Material> Engine::Containers::MaterialContainer::GetMaterial(const std::string& p_name)
{
    return GetMaterial(FindMaterial(p_name));
}
