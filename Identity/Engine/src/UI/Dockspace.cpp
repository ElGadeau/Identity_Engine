#include <stdafx.h>

#include <Tools/ImGUI/imgui.h>

#include <Core/App.h>
#include <Rendering/Renderer.h>
#include <UI/Dockspace.h>


#include <Managers/SceneManager.h>
#include <UI/imfilebrowser.h>
#include <Components/ModelComponent.h>
#include <Objects/GameObject.h>
#include <Scene/Scene.h>

void Engine::UI::Dockspace::CreateDockspace(Core::App& p_appRef)
{
    static bool               opt_fullscreen_persistant = true;
    bool                      opt_fullscreen = opt_fullscreen_persistant;
    static ImGuiDockNodeFlags dockspace_flags = ImGuiDockNodeFlags_None;

    // We are using the ImGuiWindowFlags_NoDocking flag to make the parent window not dockable into,
    // because it would be confusing to have two docking targets within each others.
    ImGuiWindowFlags window_flags = ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoDocking;
    if (opt_fullscreen)
    {
        ImGuiViewport* viewport = ImGui::GetMainViewport();
        ImGui::SetNextWindowPos(viewport->Pos);
        ImGui::SetNextWindowSize(viewport->Size);
        ImGui::SetNextWindowViewport(viewport->ID);
        ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
        ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
        window_flags |= ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize |
            ImGuiWindowFlags_NoMove;
        window_flags |= ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus | ImGuiWindowFlags_MenuBar;
    }
    if (dockspace_flags & ImGuiDockNodeFlags_PassthruCentralNode)
        window_flags |= ImGuiWindowFlags_NoBackground;
    static bool test = true;
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
    ImGui::Begin("Identity Dockspace", &test, window_flags);
    ImGui::PopStyleVar();

    if (opt_fullscreen)
        ImGui::PopStyleVar(2);

    ImGuiIO& io = ImGui::GetIO(); (void)io;


    // DockSpace
    if (io.ConfigFlags & ImGuiConfigFlags_DockingEnable)
    {
        ImGuiID dockspace_id = ImGui::GetID("Identity Dockspace");
        ImGui::DockSpace(dockspace_id, ImVec2(0.0f, 0.0f), dockspace_flags);

        CreateMenuBar(p_appRef);

    }
    ImGui::End();
}

void Engine::UI::Dockspace::CreateMenuBar(Core::App& p_appRef)
{
    static bool showSave = false;
    static bool MatCreationPopup = false;
    static bool chooseScene = false;

    if (ImGui::BeginMenuBar())
    {

        //todo all shortcuts
        if (ImGui::BeginMenu("File"))
        {
            ImGui::MenuItem("New Scene", "Ctrl + N", nullptr);
            if (ImGui::MenuItem("Open Scene", "Ctrl + O", nullptr))
            {
                UI::FileBrowser::GetInstance()->Open();
                chooseScene = true;
            }
            ImGui::Separator();
            if (ImGui::MenuItem("Save", "Ctrl + S", nullptr))
            {
                Managers::SceneManager::SaveActiveScene();
            }
            if (ImGui::MenuItem("Save As...", "Ctrl + Shift + S", nullptr))
            {
                showSave = true;
            }
            ImGui::MenuItem("Build", "Ctrl + B", nullptr);
            ImGui::Separator();
            if (ImGui::MenuItem("Exit", nullptr, nullptr))
            {
                p_appRef.SetIsApplicationRunning(false);
            }
            ImGui::EndMenu();
        }
        if (ImGui::BeginMenu("Edit"))
        {
            if (ImGui::MenuItem("Play", "Ctrl + P", nullptr))
            {
                p_appRef.TestingSimulation();
            }
            if (ImGui::MenuItem("Stop", "Ctrl + O", nullptr))
            {
                p_appRef.TestingSimulation(true);
            }
            ImGui::EndMenu();
        }

        if (ImGui::BeginMenu("Assets"))
        {
            if (ImGui::BeginMenu("Create"))
            {
                ImGui::MenuItem("Folder", "", nullptr);
                ImGui::Separator();
                ImGui::MenuItem("Scene", "", nullptr);
                ImGui::Separator();

                if (ImGui::MenuItem("Material", "", nullptr))
                {
                    MatCreationPopup = true;
                }
                ImGui::EndMenu();
            }
            ImGui::EndMenu();
        }

        if (ImGui::BeginMenu("GameObject"))
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
            ImGui::EndMenu();
        }

        ImGui::SetCursorPosX(Rendering::Renderer::GetInstance()->GetWidth() / 3);
        ImGui::Text(std::string("Scene : " + Managers::SceneManager::GetActiveScene()->GetName()).c_str());

        const ImVec2 size{ 35.0f, 0.0f };
        ImGui::SetCursorPosX((Rendering::Renderer::GetInstance()->GetWidth() / 2) - size.x);
        if (ImGui::Button("Play", size))
        {
            p_appRef.TestingSimulation();
        }

        ImGui::SetCursorPosX(Rendering::Renderer::GetInstance()->GetWidth() * 0.955f);
        if (ImGui::Button("Debug Mode"))
        {
            Systems::RenderSystem::SetDebugMode(!Systems::RenderSystem::IsDebugMode());
        }

        ImGui::EndMenuBar();
    }

    UI::FileBrowser::GetInstance()->Display();

    //show load scene dialog
    if (UI::FileBrowser::GetInstance()->HasSelected() && chooseScene)
    {
        Engine::Managers::SceneManager::LoadScene(UI::FileBrowser::GetInstance()->GetSelected());
        UI::FileBrowser::GetInstance()->ClearSelected();
        UI::FileBrowser::GetInstance()->Close();
        chooseScene = false;

    }

    //show save scene dialog
    if (showSave)
    {
        if (ImGui::Begin("Save As", &showSave, ImGuiWindowFlags_AlwaysAutoResize))
        {
            ImGui::SetWindowFocus();
            ImGui::Text("Enter scene save name: ");
            ImGui::SameLine();
            static char buf1[64] = ""; ImGui::InputText(" ", buf1, 64);
            ImGui::SameLine();

            if (ImGui::Button("Save"))
            {
                Engine::Managers::SceneManager::SaveActiveSceneAs(buf1);
                showSave = false;
            }
            ImGui::SameLine();
            if (ImGui::Button("Close"))
            {
                showSave = false;
            }
            ImGui::Text("Scene will be saved to Editor/SaveFiles/Scenes.");

            ImGui::End();
        }
    }
    if (MatCreationPopup)
    {
        ImGui::Begin("Material Creator");
        ImGui::Text("Please Choose a name");

        static char* name = new char[512];
        static std::string texName;

        ImGui::InputText("Material name", name, sizeof(name));

        if (ImGui::BeginPopup("Dock-Select Texture"))
        {
            ImGui::Text("Choose Texture");
            for (auto& texture : Managers::ResourceManager::GetAllTextures())
            {
                if (texture->GetName() == "NoName")
                    continue;

                if (ImGui::Button(texture->GetName().c_str()))
                {
                    texName = texture->GetName();
                }
            }

            ImGui::Button("Add new texture");

            ImGui::EndPopup();
        }

        if (ImGui::Button("Choose a texture"))
        {
            ImGui::OpenPopup("Dock-Select Texture");
        }
        ImGui::Separator();
        if (ImGui::Button("Create Material"))
        {
            MatCreationPopup = false;
            Managers::ResourceManager::CreateMaterial(name, "defaultPS", "defaultVS", texName);
        }
        ImGui::End();
    }
}
