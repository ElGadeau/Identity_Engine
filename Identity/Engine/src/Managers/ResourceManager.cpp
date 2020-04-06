#include <stdafx.h>
#include <Managers/ResourceManager.h>
#include <3DLoader/ObjectLoader.h>
#include <Rendering/Renderer.h>
#include <Scene/SceneGraph/SceneGraph.h>

std::unique_ptr<Engine::Managers::ResourceManager>& Engine::Managers::ResourceManager::GetInstance()
{
    if (m_instance == nullptr)
        m_instance = std::make_unique<ResourceManager>();

    return m_instance;
}

std::shared_ptr<Engine::ObjectElements::Model> Engine::Managers::ResourceManager::AddModel(const std::string& p_path,
                                                                                           const std::string& p_name)
{
    for (auto model : GetInstance()->m_models)
    {
        if (model->GetPath() == p_path)
        {
            const std::string info("The model located at " + p_path + " is already loaded and will be returned");
            MessageBox(nullptr, info.c_str(), "Info", MB_ICONINFORMATION | MB_OK);
            return model;
        }

        if (model->GetName() == p_name)
        {
            const std::string info("The name '" + p_name + "' is already in use, please change the name");
            MessageBox(nullptr, info.c_str(), "Error", MB_ICONERROR | MB_OK);
            return nullptr;
        }
    }

    std::shared_ptr<ObjectElements::Model> model = ObjectLoader::LoadModel(p_path);

    if (model == nullptr)
    {
        const std::string
                error("ResourceManager::AddModel(const std::string& p_path, const std::string& p_name): Could not load model at "
                      + p_path + " because there was no object to be found at that path");
        MessageBox(nullptr, error.c_str(), "Error", MB_ICONWARNING | MB_OK);
        return nullptr;
    }

    model->SetName(p_name);
    model->SetPath(p_path);

    for (auto& mesh : model->GetMeshes())
        mesh->GenerateBuffers(Rendering::Renderer::GetInstance()->GetDevice());

    GetInstance()->m_models.push_back(model);
    Scene::SceneGraph::GetInstance()->AddRootSceneNode(model->GetRootNode());

    return model;
}

std::shared_ptr<Engine::ObjectElements::Model> Engine::Managers::ResourceManager::GetModel(const std::string& p_name)
{
    for (auto model : GetInstance()->m_models)
    {
        if (model->GetName() == p_name)
            return model;
    }

    return nullptr;
}

std::vector<std::shared_ptr<Engine::ObjectElements::Model>> Engine::Managers::ResourceManager::GetAllModels()
{
    return GetInstance()->m_models;
}

std::shared_ptr<Engine::Rendering::Materials::Texture> Engine::Managers::ResourceManager::AddTexture(
    const std::string& p_path, const std::string& p_name)
{
    for (auto texture : GetInstance()->m_textures)
    {
        if (texture->GetPath() == p_path)
        {
            const std::string info("The texture located at " + p_path + " is already loaded and will be returned");
            MessageBox(nullptr, info.c_str(), "Info", MB_ICONINFORMATION | MB_OK);
            return texture;
        }

        if (texture->GetName() == p_name)
        {
            const std::string info("The name '" + p_name + "' is already in use, please change the name");
            MessageBox(nullptr, info.c_str(), "Error", MB_ICONERROR | MB_OK);
            return nullptr;
        }
    }

    std::shared_ptr<Rendering::Materials::Texture> texture = Rendering::Materials::Texture::LoadTexture(p_path, p_name);

    if (texture == nullptr)
    {
        const std::string
                error("ResourceManager::AddTexture(const std::string& p_path, const std::string& p_name): Could not load texture at "
                      + p_path + " because there was no object to be found at that path");
        MessageBox(nullptr, error.c_str(), "Error", MB_ICONWARNING | MB_OK);
        return nullptr;
    }

    texture->SetName(p_name);
    texture->SetPath(p_path);

    GetInstance()->m_textures.push_back(texture);

    return texture;
}

std::shared_ptr<Engine::Rendering::Materials::Texture> Engine::Managers::ResourceManager::GetTexture(
    const std::string& p_name)
{
    for (auto texture : GetInstance()->m_textures)
    {
        if (texture->GetName() == p_name)
            return texture;
    }

    return nullptr;
}

std::vector<std::shared_ptr<Engine::Rendering::Materials::Texture>> Engine::Managers::ResourceManager::GetAllTextures()
{
    return GetInstance()->m_textures;
}

std::shared_ptr<Engine::Rendering::Materials::PixelShader> Engine::Managers::ResourceManager::AddPixelShader(
    const std::string& p_path, const std::string& p_name)
{
    for (auto pShader : GetInstance()->m_pixelShaders)
    {
        if (pShader->GetPath() == p_path)
        {
            const std::string info("The pixel shader located at " + p_path + " is already loaded and will be returned");
            MessageBox(nullptr, info.c_str(), "Info", MB_ICONINFORMATION | MB_OK);
            return pShader;
        }

        if (pShader->GetName() == p_name)
        {
            const std::string info("The name '" + p_name + "' is already in use, please change the name");
            MessageBox(nullptr, info.c_str(), "Error", MB_ICONERROR | MB_OK);
            return nullptr;
        }
    }

    std::shared_ptr<Rendering::Materials::PixelShader> pShader = Rendering::Materials::PixelShader::
            LoadShader(p_path, p_name);

    if (pShader == nullptr)
    {
        const std::string
                error("ResourceManager::AddPixelShader(const std::string& p_path, const std::string& p_name): Could not load pixel shader at "
                      + p_path + " because there was no object to be found at that path");
        MessageBox(nullptr, error.c_str(), "Error", MB_ICONWARNING | MB_OK);
        return nullptr;
    }

    pShader->SetName(p_name);
    pShader->SetPath(p_path);

    GetInstance()->m_pixelShaders.push_back(pShader);

    return pShader;
}

std::shared_ptr<Engine::Rendering::Materials::PixelShader> Engine::Managers::ResourceManager::GetPixelShader(
    const std::string& p_name)
{
    for (auto pShader : GetInstance()->m_pixelShaders)
    {
        if (pShader->GetName() == p_name)
            return pShader;
    }

    return nullptr;
}

std::vector<std::shared_ptr<Engine::Rendering::Materials::PixelShader>> Engine::Managers::ResourceManager::
GetAllPixelShaders()
{
    return GetInstance()->m_pixelShaders;
}

std::shared_ptr<Engine::Rendering::Materials::VertexShader> Engine::Managers::ResourceManager::AddVertexShader(
    const std::string& p_path, const std::string& p_name)
{
    for (auto vShader : GetInstance()->m_vertexShaders)
    {
        if (vShader->GetPath() == p_path)
        {
            const std::string
                    info("The vertex shader located at " + p_path + " is already loaded and will be returned");
            MessageBox(nullptr, info.c_str(), "Info", MB_ICONINFORMATION | MB_OK);
            return vShader;
        }

        if (vShader->GetName() == p_name)
        {
            const std::string info("The name '" + p_name + "' is already in use, please change the name");
            MessageBox(nullptr, info.c_str(), "Error", MB_ICONERROR | MB_OK);
            return nullptr;
        }
    }

    std::shared_ptr<Rendering::Materials::VertexShader> vShader = Rendering::Materials::VertexShader::
            LoadShader(p_path, p_name);

    if (vShader == nullptr)
    {
        const std::string
                error("ResourceManager::AddVertexShader(const std::string& p_path, const std::string& p_name): Could not load vertex shader at "
                      + p_path + " because there was no object to be found at that path");
        MessageBox(nullptr, error.c_str(), "Error", MB_ICONWARNING | MB_OK);
        return nullptr;
    }

    vShader->SetName(p_name);
    vShader->SetPath(p_path);

    GetInstance()->m_vertexShaders.push_back(vShader);

    return vShader;
}

std::shared_ptr<Engine::Rendering::Materials::VertexShader> Engine::Managers::ResourceManager::GetVertexShader(
    const std::string& p_name)
{
    for (auto vShader : GetInstance()->m_vertexShaders)
    {
        if (vShader->GetName() == p_name)
            return vShader;
    }

    return nullptr;
}

std::vector<std::shared_ptr<Engine::Rendering::Materials::VertexShader>> Engine::Managers::ResourceManager::
GetAllVertexShaders()
{
    return GetInstance()->m_vertexShaders;
}

std::shared_ptr<Engine::Rendering::Materials::Material> Engine::Managers::ResourceManager::CreateMaterial(
    const std::string& p_name, const std::string& p_pixelShaderName, const std::string& p_vertexShaderName,
    const std::string& p_textureName)
{
    //TODO when serialization is complete: export the new material in a file
    for (auto material: GetInstance()->m_materials)
    {
        if (material->GetName() == p_name)
        {
            const std::string info("The name '" + p_name + "' is already in use, please change the name");
            MessageBox(nullptr, info.c_str(), "Error", MB_ICONERROR | MB_OK);
            return nullptr;
        }
    }

    if (GetInstance()->GetPixelShader(p_pixelShaderName) == nullptr)
    {
        const std::string error("The pixel shader you provided does not exist");
        MessageBox(nullptr, error.c_str(), "Error", MB_ICONWARNING | MB_OK);
        return nullptr;
    }

    if (GetInstance()->GetVertexShader(p_vertexShaderName) == nullptr)
    {
        const std::string error("The vertex shader you provided does not exist");
        MessageBox(nullptr, error.c_str(), "Error", MB_ICONWARNING | MB_OK);
        return nullptr;
    }

    if (p_name.length() > 0 && GetInstance()->GetTexture(p_textureName) == nullptr)
    {
        const std::string error("The texture you provided does not exist");
        MessageBox(nullptr, error.c_str(), "Error", MB_ICONWARNING | MB_OK);
        return nullptr;
    }

    std::shared_ptr<Rendering::Materials::Material> material = Rendering::Materials::Material::CreateMaterial(GetInstance()->GetPixelShader(p_pixelShaderName),
                                                                                                             GetInstance()->GetVertexShader(p_vertexShaderName));

    if (material == nullptr)
    {
        const std::string error("There was an error in the creation of the material");
        MessageBox(nullptr, error.c_str(), "Error", MB_ICONWARNING | MB_OK);
        return nullptr;
    }

    if (p_name.length() <= 0)
        material->SetTexture(GetInstance()->GetTexture(p_textureName));

    material->SetName(p_name);

    GetInstance()->m_materials.push_back(material);

    return material;
}

std::shared_ptr<Engine::Rendering::Materials::Material> Engine::Managers::ResourceManager::GetMaterial(
    const std::string& p_name)
{
    for (auto material : GetInstance()->m_materials)
    {
        if (material->GetName() == p_name)
            return material;
    }

    return nullptr;
}

std::vector<std::shared_ptr<Engine::Rendering::Materials::Material>> Engine::Managers::ResourceManager::
GetAllMaterials()
{
    return GetInstance()->m_materials;
}