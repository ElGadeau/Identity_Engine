#include <stdafx.h>

#include <windows.h>

#include <Components/BoxCollider.h>
#include <Components/SphereCollider.h>
#include <filesystem>
#include <Managers/SceneManager.h>
#include <Scene/Scene.h>

#include "Components/Light.h"
#include "Components/Sound.h"


Engine::Managers::SceneManager::SceneManager()
{
    auto dfscene = std::make_shared<Scene::Scene>();
    dfscene->SetName("default");
    m_scenes.push_back(dfscene);
    m_activeScene = dfscene;
}

std::unique_ptr<Engine::Managers::SceneManager>& Engine::Managers::SceneManager::GetInstance()
{
    if (m_instance == nullptr)
        m_instance = std::make_unique<SceneManager>();

    return m_instance;
}

std::shared_ptr<Engine::Scene::Scene> Engine::Managers::SceneManager::GetScene(const std::string& p_name)
{
    for (auto& scene : GetInstance()->m_scenes)
    {
        if (scene->GetName() == p_name)
            return scene;
    }

    return nullptr;
}

void Engine::Managers::SceneManager::AddScene(const std::shared_ptr<Scene::Scene> p_scene)
{
    GetInstance()->m_scenes.push_back(p_scene);
}

std::shared_ptr<Engine::Scene::Scene> Engine::Managers::SceneManager::LoadScene(const std::string& p_name)
{
    auto activeScene = Managers::SceneManager::GetActiveScene();
    activeScene->ClearScene();
    auto scene = std::make_shared<Scene::Scene>();
    scene->SetName(p_name);
    AddScene(scene);
    SetActiveScene(scene);
    scene->Load(p_name);
    return scene;
}

std::shared_ptr<Engine::Scene::Scene> Engine::Managers::SceneManager::LoadScene(std::filesystem::path p_path)
{
    auto activeScene = Managers::SceneManager::GetActiveScene();
    activeScene->ClearScene();
    auto scene = std::make_shared<Scene::Scene>();
    scene->SetName(p_path.string());
    AddScene(scene);
    SetActiveScene(scene);
    scene->Load(p_path);
    return scene;
}

void Engine::Managers::SceneManager::SaveActiveScene()
{
    GetActiveScene()->Save();
}

void Engine::Managers::SceneManager::SaveActiveSceneAs(const char* p_name)
{
    GetActiveScene()->SaveAs(p_name);
}

void Engine::Managers::SceneManager::SetActiveScene(const std::string& p_name)
{
    for (auto scene : GetInstance()->m_scenes)
    {
        if (scene->GetName() == p_name)
        {
            GetInstance()->m_activeScene = scene;
            return;
        }
    }

    const std::string info("SceneManager::SetActiveScene(const std::string& p_name): Could not set active scene to " +
                           p_name + " because a scene with this name could not be found\n");
    MessageBox(nullptr, info.c_str(), "Info", MB_ICONINFORMATION | MB_OK);
}

void Engine::Managers::SceneManager::SetActiveScene(const int32_t p_id)
{
    for (auto scene : GetInstance()->m_scenes)
    {
        if (scene->GetID() == p_id)
        {
            GetInstance()->m_activeScene = scene;
            return;
        }
    }

    const std::string info("SceneManager::SetActiveScene(const int32_t p_id): Could not set active scene to " + std::
                           to_string(p_id) + " because a scene with this name could not be found\n");
    MessageBox(nullptr, info.c_str(), "Info", MB_ICONINFORMATION | MB_OK);
}

void Engine::Managers::SceneManager::SetPlayScene(const std::string& p_name)
{
    for (auto scene : GetInstance()->m_scenes)
    {
        if (scene->GetName() == p_name)
        {
            GetInstance()->m_playScene = scene;
            return;
        }
    }

    const std::string info{
        "SceneManager::SetPlayScene(const std::string& p_name): Could not set active scene to " +
        p_name + " because a scene with this name could not be found\n"
    };
    MessageBox(nullptr, info.c_str(), "Info", MB_ICONINFORMATION | MB_OK);
}

void Engine::Managers::SceneManager::SetPlayScene(const int32_t p_id)
{
    for (auto scene : GetInstance()->m_scenes)
    {
        if (scene->GetID() == p_id)
        {
            GetInstance()->m_playScene = scene;
            return;
        }
    }

    const std::string info{ "SceneManager::SetPlayScene(const int32_t p_id): Could not set active scene to " + std::
                           to_string(p_id) + " because a scene with this name could not be found\n" };
    MessageBox(nullptr, info.c_str(), "Info", MB_ICONINFORMATION | MB_OK);
}

bool Engine::Managers::SceneManager::DeletePlayScene()
{
    return GetInstance()->DeletePlaySceneNS();
}


void Engine::Managers::SceneManager::DuplicateScene(std::shared_ptr<Scene::Scene>& p_destination, std::shared_ptr<Scene::Scene>& p_source)
{
    for (auto gameObject : p_source->GetAllGameObjectsInScene())
    {
        //create new gameobject
        auto name   = gameObject->GetName();
        auto tmpOBJ = std::make_shared<Objects::GameObject>(name);
        tmpOBJ->SetActive(gameObject->IsActive());

        //we now have gameobject with same name

        //change the transform to be the same
        auto goTransform = gameObject->GetTransform();
        tmpOBJ->GetTransform()->CopyFrom(gameObject->GetTransform());

        //Find all component of each type
        for (auto component : gameObject->FindAllComponentsOfType<Components::BoxCollider>())
        {
            tmpOBJ->AddComponent<Components::BoxCollider>(component);
        }
        for (auto component : gameObject->FindAllComponentsOfType<Components::ModelComponent>())
        {
            tmpOBJ->AddComponent<Components::ModelComponent>(component);
        }
        for (auto component : gameObject->FindAllComponentsOfType<Components::SphereCollider>())
        {
            tmpOBJ->AddComponent<Components::SphereCollider>(component);
        }
        for (auto component : gameObject->FindAllComponentsOfType<Components::Sound>())
        {
            tmpOBJ->AddComponent<Components::Sound>(component);
        }
        for (auto component : gameObject->FindAllComponentsOfType<Components::Light>())
        {
            tmpOBJ->AddComponent<Components::Light>(component);
        }
        p_destination->AddGameObject(tmpOBJ);
    }
}

bool Engine::Managers::SceneManager::DeletePlaySceneNS()
{
    int id = -1;
    for (int i = 0; i < m_scenes.size(); ++i)
    {
        if (m_scenes[i] == m_playScene)
            id = i;
    }

    if (id < 0)
        return false;

    m_scenes[id]->ClearScene();
    m_playScene->ClearScene();
    return true;
}
