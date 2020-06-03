#include <stdafx.h>

#include <Tools/ImGUI/imgui.h>

#include <Core/App.h>
#include <Components/BoxCollider.h>
#include <Components/SphereCollider.h>
#include <Components/Camera.h>
#include <Components/Light.h>
#include <Components/Sound.h>
#include <Managers/SceneManager.h>
#include <Rendering/Renderer.h>
#include <Scene/Scene.h>
#include <UI/Hierarchy.h>


#include "Containers/GameObjectContainer.h"
#include "UI/imfilebrowser.h"

int Engine::UI::Hierarchy::m_currentlySelected = -1;

std::shared_ptr<Engine::Scene::SceneNode> Engine::UI::Hierarchy::DisplayNextChild(std::shared_ptr<Scene::SceneNode> p_child)
{
    if (!p_child)
        return nullptr;

    static ImGuiTreeNodeFlags base_flags = ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_SpanAvailWidth;
    static int                selection_mask = (1 << 2);
    ImGuiTreeNodeFlags        node_flags = base_flags;
    static bool               test_drag_and_drop = true;
    const bool                is_selected = (selection_mask & (1 << p_child->GetID())) != 0;


    if (is_selected)
        node_flags |= ImGuiTreeNodeFlags_Selected;

    if (!p_child->GetChildren().empty())
    {
        bool node_open = ImGui::TreeNodeEx((void*)(intptr_t)p_child->GetID(), node_flags, "%s", p_child->GetName().c_str());
        {

            if (ImGui::IsItemClicked())
                m_currentlySelected = p_child->GetID();
            else if ((ImGui::IsMouseClicked(0) || ImGui::IsMouseClicked(1)) && !ImGui::IsAnyItemHovered() && ImGui::IsWindowHovered(ImGuiHoveredFlags_RootWindow))
            {
                m_currentlySelected = -1;
                selection_mask = false;
            }

            if (test_drag_and_drop && ImGui::BeginDragDropSource())
            {
                ImGui::SetDragDropPayload("_TREENODE", NULL, 0);
                ImGui::Text("%s", p_child->GetName().c_str());
                ImGui::EndDragDropSource();
            }
            if (node_open)
            {
                for (auto& child : p_child->GetChildren())
                    DisplayNextChild(child);
                ImGui::TreePop();
            }
        }
    }
    else
    {
        node_flags |= ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_NoTreePushOnOpen; // ImGuiTreeNodeFlags_Bullet
        ImGui::TreeNodeEx((void*)(intptr_t)p_child->GetID(), node_flags, "%s", p_child->GetName().c_str());

        if (ImGui::IsItemClicked())
            m_currentlySelected = p_child->GetID();
        else if ((ImGui::IsMouseClicked(0) || ImGui::IsMouseClicked(1)) && !ImGui::IsAnyItemHovered() && ImGui::IsWindowHovered(ImGuiHoveredFlags_RootWindow))
        {
            m_currentlySelected = -1;
            selection_mask = false;
        }
    }
    if (m_currentlySelected != -1)
    {
        if (ImGui::GetIO().KeyCtrl)
            selection_mask ^= (1 << m_currentlySelected);          // CTRL+click to toggle
        else if (!(selection_mask & (1 << m_currentlySelected)))
            selection_mask = (1 << m_currentlySelected);           // Click to single-select

    }

    return p_child;
}

void Engine::UI::Hierarchy::CreateHierarchy(Core::App& p_appRef)
{
    if (ImGui::Begin("Hierarchy"))
    {
        int i = 0;
        for (auto& node : Managers::SceneManager::GetActiveScene()->GetSceneGraph().GetRootSceneNodes())
            DisplayNextChild(node.second);

        ShowMenu();
    }
    ImGui::End();
    CallInspector(m_currentlySelected);
}

void Engine::UI::Hierarchy::ShowMenu()
{

    if (m_currentlySelected > 0)
    {
        if (ImGui::BeginPopupContextWindow("Hierarchy Menu"))
        {
            auto gameObject = Managers::SceneManager::GetActiveScene()->GetSceneGraph().GetAllSceneNodes().find(m_currentlySelected)->second->GetGameObject();
            static char buf1[64] = "";
            std::string name = gameObject->GetName();

            for (int i = 0; i < name.length(); ++i)
            {
                buf1[i] = name[i];
            }

            ImGui::InputText(" ", buf1, 64);
            ImGui::SameLine();

            gameObject->SetName(buf1);

            if (gameObject->GetSceneNode())
                gameObject->GetSceneNode()->SetName(buf1);

            memset(buf1, 0, 64);

            if (ImGui::Button("Delete"))
            {
                m_currentlySelected = -1;
                Managers::SceneManager::GetActiveScene()->GetSceneGraph().RemoveGameObjectFromScene(gameObject);
            }

            ImGui::EndPopup();
        }
    }

    else if (ImGui::BeginPopupContextWindow("Hierarchy Menu"))
    {
        if (ImGui::MenuItem("Create Empty", "Ctrl + Shift + N", nullptr))
        {
            auto cube = std::make_shared<Objects::GameObject>("GameObject");
            auto scene = Managers::SceneManager::GetActiveScene();
            scene->AddGameObject(cube);
        }
        if (ImGui::BeginMenu("3D Object"))
        {
            if (ImGui::MenuItem("Cube", "", nullptr))
            {
                auto cube = std::make_shared<Objects::GameObject>("Cube");
                cube->GetTransform()->SetPosition({ 0.f, 0.f, 0.f });
                cube->AddComponent<Components::ModelComponent>("Cube");
                auto scene = Managers::SceneManager::GetActiveScene();
                scene->AddGameObject(cube);
            }
            ImGui::EndMenu();
        }

        ImGui::EndPopup();
    }
}

void Engine::UI::Hierarchy::CallInspector(int32_t p_id)
{
    static bool loadNewModel = false;

    ImGui::Begin("Inspector");
    if (p_id < 0)
    {
        ImGui::End();
        return;
    }

    auto sceneNode = Managers::SceneManager::GetActiveScene()->GetSceneGraph().GetAllSceneNodes().find(p_id)->second;
    auto gameObject = sceneNode->GetGameObject();
    auto transform = gameObject->GetTransform();

    static char buf1[64] = "";
    std::string name = gameObject->GetName();

    for (int i = 0; i < name.length(); ++i)
    {
        buf1[i] = name[i];
    }

    ImGui::InputText(" ", buf1, 64);

    if (ImGui::IsItemEdited())
        gameObject->SetName(buf1);

    if (gameObject->GetSceneNode())
        gameObject->GetSceneNode()->SetName(buf1);

    memset(buf1, 0, 64);

    bool active = gameObject->IsActive();

    ImGui::SameLine();
    if (ImGui::Checkbox("Active", &active))
    {
        gameObject->SetActive(active);
    }
    //todo Class Inspector to deal with these


    Quaternion& rotationQuaternion = transform->GetRotation();
    Vector3F    rotationEuler = { static_cast<float>(rotationQuaternion.ToEuler().x),
                                       static_cast<float>(rotationQuaternion.ToEuler().y),
                                       static_cast<float>(rotationQuaternion.ToEuler().z)
    };
    if (ImGui::CollapsingHeader("Transform"), ImGuiTreeNodeFlags_DefaultOpen)
    {
        ImGui::Dummy(ImVec2(0.0f, 2.0f));
        float* pos[3] = { &transform->GetPosition().x, &transform->GetPosition().y, &transform->GetPosition().z };
        float* rot[3] = { &rotationEuler.x, &rotationEuler.y, &rotationEuler.z };
        float* scale[3] = { &transform->GetScale().x, &transform->GetScale().y, &transform->GetScale().z };
        if (ImGui::DragFloat3("Position", *pos, 0.1f, 0, 0, "%0.2f"))
            transform->needUpdate = true;
        if (ImGui::DragFloat3("Rotation", *rot, 0.1f, 0, 0, "%0.2f"))
        {
            transform->needUpdate = true;
            transform->needAxesUpdate = true;
        }
        if (ImGui::DragFloat3("Scale", *scale, 0.1f, 0, 0, "%0.2f"))
            transform->needUpdate = true;
        ImGui::Dummy(ImVec2(0.0f, 5.0f));
    }

    if (rotationEuler.y > 90.0f || rotationEuler.y < -90.0f)
        rotationEuler = Vector3{ rotationEuler.x - 180.0f, ((rotationEuler.y) * -1), rotationEuler.z + 180.0f };

    rotationQuaternion.MakeFromEuler(rotationEuler);



    for (auto component : gameObject->GetAllComponents())
    {
        std::shared_ptr<Components::IComponent> Icomponent = Containers::ComponentContainer::FindComponent(component);

        switch (Icomponent->GetType())
        {
#pragma region Model
        case Components::MODEL:
        {
            std::shared_ptr<Components::ModelComponent> modelComponent = std::dynamic_pointer_cast<Components::ModelComponent>(Icomponent);

            if (ImGui::CollapsingHeader("Model Component", ImGuiTreeNodeFlags_DefaultOpen))
            {
                ImGui::Dummy(ImVec2(0.0f, 2.0f));

                bool compActive = Icomponent->IsActive();
                if (ImGui::Checkbox("Active##1", &compActive))
                {
                    Icomponent->SetActive(compActive);
                }

                if (const auto model = Managers::ResourceManager::FindModel(modelComponent->GetModel()))
                    ImGui::Text("Current model : %s", model->GetName().c_str());

                if (ImGui::BeginPopup("Select Mesh"))
                {
                    ImGui::Text("Choose Model");
                    for (auto& model : Managers::ResourceManager::GetAllModels())
                    {
                        if (gameObject->GetModel() != nullptr)
                        {
                            if (model->GetName()._Equal(gameObject->GetModel()->GetName()))
                                continue;
                        }

                        if (model->GetName()._Equal("NoName"))
                                continue;

                        if (ImGui::Button(model->GetName().c_str()))
                        {
                            modelComponent->SetModel(model->GetID());
                            for (auto& go : Managers::SceneManager::GetActiveScene()->GetSceneGraph().GetRootSceneNodes())
                                if (go.second->GetGameObject() == gameObject)
                                    go.second->SetModel(model);

                        }
                    }

                    if (ImGui::Button("Add new model"))
                    {
                        UI::FileBrowser::GetInstance()->Open();
                        loadNewModel = true;
                    }

                    ImGui::EndPopup();
                }

                if (ImGui::Button("Change Model"))
                {
                    ImGui::OpenPopup("Select Mesh");
                }

                ImGui::Dummy(ImVec2(0.0f, 2.0f));
                if (ImGui::Button("Remove Component##1"))
                {
                    gameObject->RemoveComponent(modelComponent->GetID());
                }
                ImGui::Dummy(ImVec2(0.0f, 5.0f));

            }
#pragma endregion
#pragma region Material
                //todo fix change material button being too low (bug, probably due to names)
            if (ImGui::CollapsingHeader("Material"), ImGuiTreeNodeFlags_DefaultOpen)
            {
                ImGui::Dummy(ImVec2(0.0f, 2.0f));
                std::shared_ptr<Rendering::Materials::Material> mat = modelComponent->GetMaterial();

                if (ImGui::BeginPopup("Select Material"))
                {
                    ImGui::Text("Choose Material");
                    if (ImGui::Button("None"))
                        modelComponent->SetMaterial("default");
                    
                    for (auto& material : Managers::ResourceManager::GetAllMaterials())
                    {
                        if (material->GetName() == "NoName" || material->GetName() == "RenderText" || material->GetName() == "default")
                            continue;

                        if (ImGui::Button(material->GetName().c_str()))
                        {
                            modelComponent->SetMaterial(material->GetName());
                        }
                    }

                    ImGui::EndPopup();
                }

                if (mat == nullptr)
                    break;

                if (mat->GetName()._Equal("default"))
                {
                    ImGui::TextWrapped("Using default material. Please add a new material to edit it.");
                    ImGui::Dummy(ImVec2(0.0f, 2.0f));
                    if (ImGui::Button("Change Material##1"))
                    {
                        ImGui::OpenPopup("Select Material");
                    }
                    break;
                }
                
                float* objectColor[3] = { &mat->GetColor().x, &mat->GetColor().y, &mat->GetColor().z };
                ImGui::ColorEdit3("Material Color", *objectColor, ImGuiColorEditFlags_PickerHueWheel | ImGuiColorEditFlags_NoInputs);
                if (mat->GetTextureState() == true)
                {
                    ImGui::Dummy(ImVec2(0.0f, 2.0f));
                    ImGui::Text("Texture");
                    ImGui::Image(*mat->GetTexture()->GetTextureShaderResourceView().GetAddressOf(), ImVec2(100, 100));
                    ImGui::Text("%s", mat->GetTexture()->GetName().c_str());
                    ImGui::Dummy(ImVec2(0.0f, 2.0f));
                }

                if (ImGui::Button("Change Material##2"))
                {
                    ImGui::OpenPopup("Select Material");
                }

                if (ImGui::BeginPopup("Select Texture"))
                {
                    ImGui::Text("Choose Texture");
                    if (ImGui::Button("None"))
                    {
                        modelComponent->GetMaterial()->SetTexture(nullptr);
                        modelComponent->GetMaterial()->SetTextureState(false);
                    }
                    for (auto& texture : Managers::ResourceManager::GetAllTextures())
                    {
                        if (texture->GetName() == "NoName")
                            continue;

                        if (ImGui::Button(texture->GetName().c_str()))
                        {
                            modelComponent->GetMaterial()->SetTexture(texture);
                        }
                    }
                    ImGui::Button("Add new texture");

                    ImGui::EndPopup();
                }

                if (ImGui::Button("Change Texture"))
                {
                    ImGui::OpenPopup("Select Texture");
                }
                ImGui::Dummy(ImVec2(0.0f, 5.0f));
            }
            break;
#pragma endregion
        }
        case Components::BOX_COLLIDER:
        {
            std::shared_ptr<Components::BoxCollider> boxCollider = std::dynamic_pointer_cast<Components::BoxCollider>(Icomponent);
            if (ImGui::CollapsingHeader("Box Collider"), ImGuiTreeNodeFlags_DefaultOpen)
            {
                ImGui::Dummy(ImVec2(0.0f, 2.0f));
                bool compActive = Icomponent->IsActive();
                if (ImGui::Checkbox("Active##2", &compActive))
                {
                    Icomponent->SetActive(compActive);
                }

                //todo add dimensions, it's weirdly done on BoxCollider for the moment being.
                float* dimensions[3] = { &boxCollider->GetDimensions().x, &boxCollider->GetDimensions().y, &boxCollider->GetDimensions().z };
                float* offset[3] = { &boxCollider->GetOffset().x, &boxCollider->GetOffset().y, &boxCollider->GetOffset().z };
                float* mass = { &boxCollider->GetMass() };

                if (ImGui::DragFloat3("Dimensions", *dimensions, 0.1f, 0, 0, "%0.2f"))
                    boxCollider->SetDimensions(boxCollider->GetDimensions());
                if (ImGui::DragFloat("Mass", mass, 0.1f, 0, 0, "%0.2f"))
                    boxCollider->SetMass(boxCollider->GetMass());
                if (ImGui::DragFloat3("Offset", *offset, 0.1f, 0, 0, "%0.2f"))
                    boxCollider->SetPositionOffset(boxCollider->GetOffset());


                ImGui::Dummy(ImVec2(0.0f, 2.0f));
                if (ImGui::Button("Remove Component##2"))
                {
                    gameObject->RemoveComponent(boxCollider->GetID());
                }
                ImGui::Dummy(ImVec2(0.0f, 5.0f));
            }
            break;
        }
        case Components::SPHERE_COLLIDER:
        {
            std::shared_ptr<Components::SphereCollider> sphereCollider = std::dynamic_pointer_cast<Components::SphereCollider>(Icomponent);
            if (ImGui::CollapsingHeader("Sphere Collider"), ImGuiTreeNodeFlags_DefaultOpen)
            {
                ImGui::Dummy(ImVec2(0.0f, 2.0f));
                bool compActive = Icomponent->IsActive();
                if (ImGui::Checkbox("Active##3", &compActive))
                {
                    Icomponent->SetActive(compActive);
                }

                //todo add dimensions, it's weirdly done on BoxCollider for the moment being.
                float* radius = { &sphereCollider->GetRadius() };
                float* offset[3] = { &sphereCollider->GetOffset().x, &sphereCollider->GetOffset().y, &sphereCollider->GetOffset().z };
                float* mass = { &sphereCollider->GetMass() };

                if (ImGui::DragFloat("Radius", radius, 0.1f, 0, 0, "%0.2f"))
                    sphereCollider->SetRadius(sphereCollider->GetRadius());
                if (ImGui::DragFloat("Mass", mass, 0.1f, 0, 0, "%0.2f"))
                    sphereCollider->SetMass(sphereCollider->GetMass());
                if (ImGui::DragFloat3("Offset", *offset, 0.1f, 0, 0, "%0.2f"))
                    sphereCollider->SetPositionOffset(sphereCollider->GetOffset());


                ImGui::Dummy(ImVec2(0.0f, 2.0f));
                if (ImGui::Button("Remove Component##3"))
                {
                    gameObject->RemoveComponent(sphereCollider->GetID());
                }
                ImGui::Dummy(ImVec2(0.0f, 5.0f));
            }
            break;
        }
        case Components::CAMERA:
        {
            std::shared_ptr<Components::Camera> camera = std::dynamic_pointer_cast<Components::Camera>(Icomponent);
            if (ImGui::CollapsingHeader("Camera"), ImGuiTreeNodeFlags_DefaultOpen)
            {
                ImGui::Dummy(ImVec2(0.0f, 2.0f));
                bool compActive = Icomponent->IsActive();
                if (ImGui::Checkbox("Active##4", &compActive))
                {
                    Icomponent->SetActive(compActive);
                }

                float fov = camera->GetFOV();

                ImGui::SliderFloat("Camera FOV", &fov, 10.f, 180.f, "%0.f");

                if (ImGui::IsItemEdited())
                    camera->SetFOV(fov);

                ImGui::Dummy(ImVec2(0.0f, 2.0f));
                if (ImGui::Button("Remove Component##4"))
                {
                    gameObject->RemoveComponent(camera->GetID());
                }
                ImGui::Dummy(ImVec2(0.0f, 5.0f));
            }
            break;
        }
        case Components::LIGHT:
        {
            std::shared_ptr<Components::Light> lightComp = std::dynamic_pointer_cast<Components::Light>(Icomponent);
            std::shared_ptr<Rendering::Lights::ILight> ILight = lightComp->GetLight();
            Rendering::Lights::ILight::LightData& lightData = ILight->GetLightData();
            if (ImGui::CollapsingHeader("Light"), ImGuiTreeNodeFlags_DefaultOpen)
            {
                ImGui::Dummy(ImVec2(0.0f, 2.0f));
                bool compActive = Icomponent->IsActive();
                if (ImGui::Checkbox("Active##5", &compActive))
                {
                    Icomponent->SetActive(compActive);
                }

                float* diffuse[4] = { &lightData.diffuse.x, &lightData.diffuse.y, &lightData.diffuse.z, &lightData.diffuse.w };
                float* specular[4] = { &lightData.specular.x, &lightData.specular.y, &lightData.specular.z, &lightData.specular.w };
                float* range = { &lightData.range };
                float* shininess = { &lightData.shininess };

                ImGui::ColorEdit3("Diffuse Light Color", *diffuse, ImGuiColorEditFlags_PickerHueWheel | ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_NoLabel);
                ImGui::SetNextItemWidth(100);
                ImGui::DragFloat("Diffuse intensity", &lightData.diffuse.w, 0.1f, 0.0f, 25.00f, "%.3f");
                ImGui::Separator();
                ImGui::ColorEdit3("Specular Light Color", *specular, ImGuiColorEditFlags_PickerHueWheel | ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_NoLabel);
                ImGui::SetNextItemWidth(100);
                ImGui::SliderFloat("Specular intensity", &lightData.specular.w, 0.00f, 1.0f, "%.2f");
                ImGui::Separator();
                ImGui::SetNextItemWidth(100);
                ImGui::SliderFloat("Shininess", shininess, 8.0f, 512.0f, "%.0f");
                ImGui::SliderFloat("Range", range, 0.00f, 5.0f, "%.2f");

                ImGui::Dummy(ImVec2(0.0f, 2.0f));
                if (ImGui::Button("Remove Component##5"))
                {
                    gameObject->RemoveComponent(lightComp ->GetID());
                }
                ImGui::Dummy(ImVec2(0.0f, 5.0f));
            }
            break;
        }
        case Components::SOUND:
        {
            static bool chooseSound = false;

            std::shared_ptr<Components::Sound> sound = std::dynamic_pointer_cast<Components::Sound>(Icomponent);
            if (ImGui::CollapsingHeader("Sound"), ImGuiTreeNodeFlags_DefaultOpen)
            {
                ImGui::Dummy(ImVec2(0.0f, 2.0f));
                bool compActive = Icomponent->IsActive();
                if (ImGui::Checkbox("Active##4", &compActive))
                {
                    Icomponent->SetActive(compActive);
                }

                bool playLooped = sound->GetPlayLooped();

                if (ImGui::Checkbox("Play Looped", &playLooped))
                {
                    sound->SetPlayLooped(playLooped);
                }

                bool playPaused = sound->GetStartPaused();

                if (ImGui::Checkbox("Start Paused", &playPaused))
                {
                    sound->SetStartPaused(playPaused);
                }

                float volume = sound->GetVolume();
                ImGui::DragFloat("Volume", &volume, 0.05f, 0.f, 10.0f, "%0.3f");
                sound->SetVolume(volume);

                bool play3D = sound->GetPlaySoundIn3D();

                if (ImGui::Checkbox("Play in 3D", &play3D))
                {
                    sound->SetPlaySoundIn3D(play3D);                  
                }
                if (play3D)
                {
                    float minDist = sound->GetMinDistance();
                    float maxDist = sound->GetMaxDistance();

                    ImGui::DragFloat("Minimum Distance", &minDist, 0.2f, 0, 0, "%0.2f");
                    sound->SetMinDistance(minDist);

                    ImGui::DragFloat("Maximum Distance", &maxDist, 0.2f, 0, 0, "%0.2f");
                    sound->SetMaxDistance(maxDist);
                }


                ImGui::Text("Sound file: ");
                ImGui::Text(std::string(sound->GetFilePath()).c_str());

                if (ImGui::Button("Set Sound Source"))
                {
                    chooseSound = true;
                    UI::FileBrowser::GetInstance()->Open();
                }

                if (UI::FileBrowser::GetInstance()->HasSelected() && chooseSound)
                {
                    sound->SetSoundFile(UI::FileBrowser::GetInstance()->GetSelected());
                    UI::FileBrowser::GetInstance()->ClearSelected();
                    UI::FileBrowser::GetInstance()->Close();
                    chooseSound = false;
                }

                ImGui::Dummy(ImVec2(0.0f, 2.0f));
                if (ImGui::Button("Remove Component##6"))
                {
                    gameObject->RemoveComponent(sound->GetID());
                }
                ImGui::Dummy(ImVec2(0.0f, 5.0f));
            }
            break;
        }
        case Components::UNSET:
        {
            OutputDebugString("Component Type not Set for Inspector Call. Please set the Component Type on your Component to something valid.");
            break;
        }
        }
    }
    Vector2F windowSize{ ImGui::GetWindowSize().x, ImGui::GetWindowSize().y };
    ImGui::SetCursorPosX((windowSize.x - 200) * 0.5f);
    ImGui::SetCursorPosY(ImGui::GetCursorPosY() + 10.0f);

    if (ImGui::BeginPopup("Add Component"))
    {
        std::vector<std::string> types{ "BOX_COLLIDER",
                                        "LIGHT",
                                        "CAMERA",
                                        "MODEL",
                                        "SOUND",
                                        "SPHERE_COLLIDER" };
        for (unsigned int i = 0; i < types.size(); ++i)
        {
            if (ImGui::Button(types[i].c_str(), ImVec2(200, 20)))
            {
                const Vector2F size(1920, 1080);
                switch (i)
                {
                case 0:
                    gameObject->AddComponent<Components::BoxCollider>();
                    break;
                case 1:
                    gameObject->AddComponent<Components::Light>();
                    break;
                case 2:
                    gameObject->AddComponent<Components::Camera>(size.x, size.y);
                    break;
                case 3:
                    gameObject->AddComponent<Components::ModelComponent>();
                    break;
                case 4:
                    gameObject->AddComponent<Components::Sound>();
                    break;
                case 5:
                    gameObject->AddComponent<Components::SphereCollider>();
                    break;
                }
            }
        }
        ImGui::EndPopup();
    }

    if (ImGui::Button("Add component", ImVec2(200, 20)))
    {
        ImGui::SetCursorPosX(ImGui::GetWindowPos().x);
        ImGui::OpenPopup("Add Component");
    }

    ImGui::End();

    //if loading new model
    if (UI::FileBrowser::GetInstance()->HasSelected() && loadNewModel)
    {
        if (ImGui::Begin("New Model Name", &loadNewModel, ImGuiWindowFlags_AlwaysAutoResize))
        {
            ImGui::SetWindowFocus();
            ImGui::Text("Enter new model name: ");
            ImGui::SameLine();
            static char buf1[64] = ""; ImGui::InputText(" ", buf1, 64);
            ImGui::SameLine();

            if (ImGui::Button("Save"))
            {
                Managers::ResourceManager::AddModel(UI::FileBrowser::GetInstance()->GetSelected().string(), buf1);
                loadNewModel = false;
            }
            ImGui::SameLine();
            if (ImGui::Button("Close"))
            {
                loadNewModel = false;
            }

            ImGui::End();
        }
    }
        
    
}
