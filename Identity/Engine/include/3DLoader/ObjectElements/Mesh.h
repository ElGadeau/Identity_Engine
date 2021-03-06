#pragma once
#include <Export.h>
#include <vector>
#include <Geometry/Vertex.h>
#include <WinSetup.h>
#include <d3d11.h>
#include <Rendering/Buffers/VertexBuffer.h>
#include <Rendering/Buffers/IndexBuffer.h>
#include <Objects/IObject.h>
#include <Rendering/Buffers/InputLayout.h>

#include <Rendering/Materials/Material.h>
#include <Containers/MaterialContainer.h>

namespace Engine::ObjectElements
{
    class API_ENGINE Mesh : public Objects::IObject
    {
    public:
        Mesh() = default;
        Mesh(std::vector<Geometry::Vertex>& p_vertices, std::vector<unsigned short>& p_indices);
        ~Mesh() = default;
        Mesh(const Mesh& p_other);

        void GenerateBuffers(const Microsoft::WRL::ComPtr<ID3D11Device>& p_device);
        void Bind(const Microsoft::WRL::ComPtr<ID3D11DeviceContext>& p_context);
        void Unbind(const Microsoft::WRL::ComPtr<ID3D11DeviceContext>& p_context);
        void SetMaterial(std::shared_ptr<Rendering::Materials::Material> p_material);

        bool operator==(const Mesh& p_other) const;
        bool operator!=(const Mesh& p_other) const;

        Rendering::Buffers::VertexBuffer& GetVertexBuffer() { return m_vertexBuffer; }
        [[nodiscard]] Rendering::Buffers::IndexBuffer& GetIndexBuffer() { return m_indexBuffer; }
        [[nodiscard]] std::vector<Geometry::Vertex>& GetVertices() { return m_vertices; }
        [[nodiscard]] std::vector<unsigned short>& GetIndices() { return m_indices; }
        [[nodiscard]] std::shared_ptr<Rendering::Materials::Material> GetMaterial() { return m_material; }


        void SetTransform(int32_t p_transform) { m_transform = p_transform; }

    private:
        //buffers
        Rendering::Buffers::VertexBuffer m_vertexBuffer;
        Rendering::Buffers::IndexBuffer m_indexBuffer;
        Rendering::Buffers::InputLayout m_inputLayout;

        std::shared_ptr<Rendering::Materials::Material> m_material{ nullptr };

        //data
        std::vector<Geometry::Vertex> m_vertices;
        std::vector<unsigned short> m_indices;

        int32_t m_transform;
    };
}
