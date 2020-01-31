#pragma once

#include <Export.h>
#include <iostream>
#include <memory>
#include <3DLoader/ObjectElements/Mesh.h>
#include <3DLoader/ObjectElements/Model.h>


namespace Engine
{
    /**
     * @brief static class that uses an outside dependency of our choice to load object files
     */
    class API_ENGINE ObjectLoader
    {
    public:
        ObjectLoader() = delete;
        ~ObjectLoader() = delete;
        ObjectLoader(const ObjectLoader&) = delete;

        static std::shared_ptr<ObjectElements::Model> LoadModel(const std::string& p_file);

    private:
        static std::shared_ptr<ObjectElements::Mesh> LoadMesh(aiMesh* p_assimpMesh);
    };
}